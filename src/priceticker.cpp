#include "priceticker.h"

#include <QCoreApplication> // For qApp->applicationName() maybe?
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug> // For logging (use sparingly)

#include <Plasma/Theme> // For potential theme integration if needed

// Define static constants
const QString PriceTicker::BINANCE_WS_URL = QStringLiteral("wss://stream.binance.com:9443/ws");
const QString PriceTicker::SYMBOL = QStringLiteral("zecusdt");
const QString PriceTicker::STREAM_NAME = QStringLiteral("%1@miniTicker").arg(PriceTicker::SYMBOL);

// Constructor
PriceTicker::PriceTicker(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), // Initialize base class
      m_url(BINANCE_WS_URL),
      m_reconnectTimer(this), // Parent timer to this object
      m_currentPrice(QStringLiteral("Loading...")),
      m_connectionStatus(QStringLiteral("Disconnected")),
      m_statusColor(Qt::GlobalColor::yellow), // Initial color
      m_currentReconnectInterval(RECONNECT_INTERVAL_BASE_MS)
{
    // Configure the reconnect timer
    m_reconnectTimer.setSingleShot(true);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &PriceTicker::tryReconnect);

    // Connect WebSocket signals to our slots
    connect(&m_webSocket, &QWebSocket::connected, this, &PriceTicker::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &PriceTicker::onDisconnected);
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &PriceTicker::onTextMessageReceived);
    connect(&m_webSocket, &QWebSocket::errorOccurred, this, &PriceTicker::onErrorOccurred);
}

// Destructor
PriceTicker::~PriceTicker()
{
    // QWebSocket will be cleaned up automatically due to parent-child relationship
    // if it were dynamically allocated, but here it's a member, so ensure closure.
    if (m_webSocket.isValid()) {
        m_webSocket.close(QWebSocketProtocol::CloseCodeNormal, QStringLiteral("Widget closing"));
    }
}

// Initialization called by Plasma
void PriceTicker::init()
{
    // Set the main QML file for the applet's UI
    // The path is relative to the package structure installation path
    setMainQmlSource(QUrl::fromLocalFile(QStringLiteral("contents/ui/main.qml")));

    // Start the connection process
    connectSocket();
}

// --- Property Getters ---
QString PriceTicker::currentPrice() const { return m_currentPrice; }
QString PriceTicker::connectionStatus() const { return m_connectionStatus; }
QColor PriceTicker::statusColor() const { return m_statusColor; }

// --- Private Helper Methods ---
void PriceTicker::updateStatus(const QString &status, const QColor &color)
{
    bool changed = false;
    if (m_connectionStatus != status) {
        m_connectionStatus = status;
        qDebug() << "Status Update:" << status; // Use qDebug for dev logging
        changed = true;
        emit connectionStatusChanged();
    }
    if (m_statusColor != color) {
        m_statusColor = color;
        changed = true; // May already be true, but check color separately
        emit statusColorChanged();
    }
}

void PriceTicker::updatePrice(const QString &priceStr)
{
    bool ok;
    double price = priceStr.toDouble(&ok);
    if (ok) {
        QString formattedPrice = QStringLiteral("%L1 USDT").arg(price, 0, 'f', 2); // Localized formatting
        if (m_currentPrice != formattedPrice) {
            m_currentPrice = formattedPrice;
            emit currentPriceChanged();
        }
    } else {
        qWarning() << "Failed to parse price:" << priceStr;
        // Optionally update status or price display to indicate error
    }
}

void PriceTicker::connectSocket()
{
    if (m_webSocket.state() == QAbstractSocket::UnconnectedState) {
        qDebug() << "Attempting to connect to" << m_url.toString();
        updateStatus(QStringLiteral("Connecting..."), Qt::GlobalColor::blue);
        m_webSocket.open(m_url);
    } else if (m_webSocket.state() == QAbstractSocket::ConnectingState) {
         qDebug() << "Connection already in progress.";
         updateStatus(QStringLiteral("Connecting..."), Qt::GlobalColor::blue); // Ensure status
    } else if (m_webSocket.state() == QAbstractSocket::ConnectedState) {
         qDebug() << "Already connected.";
         updateStatus(QStringLiteral("Connected"), Qt::GlobalColor::green); // Ensure status
    }
}

void PriceTicker::scheduleReconnect()
{
    if (!m_reconnectTimer.isActive()) {
        qDebug() << "Scheduling reconnect in" << m_currentReconnectInterval << "ms";
        updateStatus(QStringLiteral("Reconnecting in %1s...").arg(m_currentReconnectInterval / 1000),
                     Qt::GlobalColor::yellow);
        m_reconnectTimer.start(m_currentReconnectInterval);
        // Exponential backoff
        m_currentReconnectInterval = std::min(m_currentReconnectInterval * 2, MAX_RECONNECT_INTERVAL_MS);
    }
}

void PriceTicker::resetReconnectTimer()
{
    m_reconnectTimer.stop();
    m_currentReconnectInterval = RECONNECT_INTERVAL_BASE_MS;
}

// --- WebSocket Slots ---
void PriceTicker::onConnected()
{
    qDebug() << "WebSocket connected.";
    updateStatus(QStringLiteral("Connected"), Qt::GlobalColor::green);
    resetReconnectTimer();

    // Subscribe to the stream
    QJsonObject payload;
    payload[QStringLiteral("method")] = QStringLiteral("SUBSCRIBE");
    QJsonArray params;
    params.append(STREAM_NAME);
    payload[QStringLiteral("params")] = params;
    payload[QStringLiteral("id")] = 1;

    m_webSocket.sendTextMessage(QString::fromUtf8(QJsonDocument(payload).toJson(QJsonDocument::Compact)));
    qDebug() << "Subscribed to" << STREAM_NAME;
}

void PriceTicker::onDisconnected()
{
    qDebug() << "WebSocket disconnected.";
    // Don't immediately set status if we are trying to reconnect
    if (!m_reconnectTimer.isActive()) {
        updatePrice(QStringLiteral("N/A"));
        updateStatus(QStringLiteral("Disconnected"), Qt::GlobalColor::red); // Explicitly disconnected
        scheduleReconnect(); // Schedule if not already planned (e.g., by onError)
    } else {
         // If timer is active, status is already "Reconnecting..."
    }
}

void PriceTicker::onTextMessageReceived(const QString &message)
{
     //qDebug() << "Message received:" << message.left(100); // Debug
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);

    if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject data = doc.object();
        if (data.contains(QStringLiteral("e")) && data[QStringLiteral("e")].toString() == QStringLiteral("24hrMiniTicker") && data.contains(QStringLiteral("c"))) {
             updatePrice(data[QStringLiteral("c")].toString());
             // Ensure status shows connected if we receive data
            if (m_connectionStatus != QStringLiteral("Connected")) {
                 updateStatus(QStringLiteral("Connected"), Qt::GlobalColor::green);
                 resetReconnectTimer(); // Got data, connection is healthy
            }
        } else {
             //qDebug() << "Received unexpected message format:" << message.left(100);
        }
    } else {
        qWarning() << "Failed to parse JSON:" << parseError.errorString() << "in message:" << message.left(100);
    }
}

void PriceTicker::onErrorOccurred(QAbstractSocket::SocketError error)
{
    qWarning() << "WebSocket error occurred:" << m_webSocket.errorString() << "(Code:" << error << ")";
    updateStatus(QStringLiteral("Error: %1").arg(m_webSocket.errorString()), Qt::GlobalColor::red);
    // Defensive close - QWebSocket might handle this, but explicit can't hurt
     if (m_webSocket.state() != QAbstractSocket::UnconnectedState) {
         m_webSocket.close();
     }
    scheduleReconnect(); // Always try to reconnect after an error
}

// --- Plugin Registration ---
#include <KPluginFactory> // Needed for plugin registration

// This macro registers the PriceTicker class with Plasma.
// The second argument ("com_example_cpppriceticker_plugin") MUST match X-KDE-PluginInfo-Name in metadata.json
K_PLUGIN_FACTORY_WITH_JSON(PriceTickerFactory, "metadata.json", registerPlugin<PriceTicker>("com_example_cpppriceticker_plugin"))

#include "priceticker.moc" // Include Meta-Object Compiler output (CMake handles generation)