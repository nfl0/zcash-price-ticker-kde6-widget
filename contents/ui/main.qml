import QtQuick 6.5 // Or your Qt6 version
import QtQuick.Layouts 1.15
import org.kde.plasma.plasmoid 2.0
import org.kde.kirigami 2.20 as Kirigami // Use KF6 Kirigami

// The C++ backend is accessed via plasmoid.nativeInterface
// Property names must match those defined with Q_PROPERTY in C++

PlasmoidItem {
    id: root
    preferredRepresentation: Plasmoid.compactRepresentation

    // --- UI Layout ---
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.smallSpacing

        Kirigami.Heading {
            id: titleLabel
            Layout.fillWidth: true
            level: 3 // Smaller heading
            text: "ZEC/USDT Price (C++)"
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Kirigami.Heading {
            id: priceLabel
            Layout.fillWidth: true
            level: 1 // Larger heading for the price
            // Access C++ property via nativeInterface
            text: plasmoid.nativeInterface.currentPrice
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 14 // Adjust size as needed
            wrapMode: Text.NoWrap
            elide: Text.ElideRight
        }

        Kirigami.Separator {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
        }

        Kirigami.Label {
            id: statusLabel
            Layout.fillWidth: true
            // Access C++ property via nativeInterface
            text: plasmoid.nativeInterface.connectionStatus
            horizontalAlignment: Text.AlignHCenter
            font.italic: true
            // Access C++ property via nativeInterface
            color: plasmoid.nativeInterface.statusColor
            wrapMode: Text.WordWrap
        }
    }

    // Optional: Call a C++ method if needed on load, though init() in C++ handles startup
    // Component.onCompleted: {
    //     // plasmoid.nativeInterface.someMethodIfNeeded()
    // }
}
