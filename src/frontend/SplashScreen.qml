// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


import QtQuick 2.0


Rectangle {
    color: "#222"
    anchors.fill: parent

    property bool scanningMeta: api.meta.isScanningMeta
    onScanningMetaChanged: {
        label_gamesState.text += "  \u2713"
        label_metaState.opacity = 1.0;
        loading_dots.anchors.left = label_metaState.right
        loading_dots.anchors.baseline = label_metaState.baseline
        scanningMeta = true;
    }

    Image {
        id: logo
        source: "/loading.svg"
        sourceSize.height: parent.height * 0.85
        opacity: 0.25

        anchors.left: parent.left
        anchors.bottom: parent.bottom
    }

    Item {
        anchors.left: parent.left
        anchors.leftMargin: parent.width * 0.61
        anchors.verticalCenter: parent.verticalCenter
        width: childrenRect.width
        height: childrenRect.height

        Text {
            id: label_found
            text: qsTr("games\nfound") + api.tr
            color: "#888"
            font {
                pixelSize: vpx(20)
                family: globalFonts.sans
                italic: true
            }

            anchors.left: counter.right
            anchors.bottom: counter.baseline
            anchors.leftMargin: vpx(8)
            anchors.bottomMargin: -vpx(6)
        }

        Text {
            id: counter

            text: api.meta.gameCount
            color: "#eaeaea"
            font {
                pixelSize: vpx(110)
                family: globalFonts.sans
            }
        }

        Text {
            id: label_gamesState

            property int leftMarginOffset: vpx(50)
            PropertyAnimation on leftMarginOffset {
                duration: 300; to: 0
                easing.type: Easing.OutQuad
            }

            text: qsTr("looking for games") + api.tr
            color: "#aaa"
            font {
                pixelSize: vpx(24)
                family: globalFonts.sans
                italic: true
            }

            anchors.left: parent.left
            anchors.top: counter.baseline
            anchors.topMargin: vpx(24)
            anchors.leftMargin: vpx(18) + leftMarginOffset

            opacity: 1.0
            Behavior on opacity { PropertyAnimation { duration: 100 } }
        }

        Text {
            id: label_metaState

            property int leftMarginOffset: vpx(50)
            PropertyAnimation on leftMarginOffset {
                duration: 400; to: 0
                easing.type: Easing.OutQuad
            }

            text: qsTr("looking for metadata") + api.tr
            color: "#aaa"
            font {
                pixelSize: vpx(24)
                family: globalFonts.sans
                italic: true
            }

            anchors.left: parent.left
            anchors.top: label_gamesState.bottom
            anchors.topMargin: vpx(4)
            anchors.leftMargin: vpx(18) + leftMarginOffset

            opacity: 0.33
            Behavior on opacity { PropertyAnimation { duration: 100 } }
        }

        Timer {
            interval: 250; running: true; repeat: true
            onTriggered: loading_dots.text = loading_dots.text.length === 3
                         ? "." : loading_dots.text + "."
        }

        Text {
            id: loading_dots
            text: "."
            color: "#aaa"
            font {
                pixelSize: vpx(24)
                family: globalFonts.sans
                italic: true
            }
            anchors.left: label_gamesState.right
            anchors.baseline: label_gamesState.baseline
        }
    }
}