#ifndef PRICETICKER_H
#define PRICETICKER_H

#include <Plasma/Applet> // Base class for Plasma applets
#include <QObject>
#include <QString>
#include <QUrl>
#include <QTimer>
#include <QWebSocket>
#include <QColor>

// Forward declarations
class QWebSocket;

class PriceTicker : public Plasma::Applet
{
    Q_OBJECT // Macro for classes using signals/slots

    // Properties exposed to QML via nativeInterface
    Q_PROPERTY(QString currentPrice READ currentPrice NOTIFY currentPriceChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(QColor statusColor READ statusColor NOTIFY statusColorChanged)

public:
    // Constructor: parent is the Plasmoid object provided by Plasma
    explicit PriceTicker(QObject *parent, const QVariantList &args);
    ~PriceTicker() override; // Destructor

    // Called by Plasma when the applet is initialized
    void init() override;

    // Property getter methods
    QString currentPrice() const;
    QString connectionStatus() const;
    QColor statusColor() const;

signals:
    // Signals emitted when properties change, updating QML bindings
    void currentPriceChanged();
    void connectionStatusChanged();
    void statusColorChanged();

private slots:
    // Slots to handle QWebSocket signals
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onErrorOccurred(QAbstractSocket::SocketError error);

    // Slot for the reconnect timer
    void tryReconnect();

private:
    // Helper methods
    void updateStatus(const QString &status, const QColor &color);
    void updatePrice(const QString &priceStr);
    void connectSocket();
    void scheduleReconnect();
    void resetReconnectTimer();

    // Member variables
    QWebSocket m_webSocket;
    QUrl m_url;
    QTimer m_reconnectTimer;

    QString m_currentPrice;
    QString m_connectionStatus;
    QColor m_statusColor;
    int m_currentReconnectInterval;

    // Configuration constants
    static const QString BINANCE_WS_URL;
    static const QString SYMBOL;
    static const QString STREAM_NAME;
    static const int RECONNECT_INTERVAL_BASE_MS = 5000;
    static const int MAX_RECONNECT_INTERVAL_MS = 60000;
};

#endif // PRICETICKER_H