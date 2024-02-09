import QtQuick 2.0

Item {
    Image{
        source: "ReviewScreen.png"
        id: reviewScreen
        anchors.fill: parent
    }
    Image{
        id: backIcon
        x: 77
        y: 13
        width: 75
        height: 75
        source: "qrc:/UI/Assets/back_arrow.png"

        MouseArea{
            anchors.fill : parent
            onClicked:{
                console.log("Back Buttton Pressed")
                reviewlist.clear()
                mainLoader.source = "../HomeScreen/HomeScreen.qml"
            }
        }
    }

    ListView {
        id: sessionsListView
        width: 1006 ; height: 300
        x: 10
        y: 229
        spacing: 13
        model: reviewlist
        delegate:
            Rectangle{
                id: mainButton
                width: sessionsListView.width
                height: sessionsListView.height / 4
                color: "transparent"

            Rectangle{
                id: dateRect
                width: 235
                height: sessionsListView.height / 4
                color: "black"
                Text {
                   id: dateText
                   anchors {
                       verticalCenter: parent.verticalCenter
                       left: parent.left
                       leftMargin: 12
                   }
                   color: "white"
                   font.pixelSize: 36
                   text: date
                }
            }
            Rectangle{
                id: sessionRect
                width: 191
                height: sessionsListView.height / 4
                color: "black"
                anchors{
                    verticalCenter: parent.verticalCenter
                    left: dateRect.right
                    leftMargin: 27
                }
                Text {
                   id: sessionText
                   anchors {
                       verticalCenter: parent.verticalCenter
                       left: parent.left
                       leftMargin: 40
                   }
                   color: "white"
                   font.pixelSize: 36
                   text: session
                }
            }
            Rectangle{
                id: lapsRect
                width: 129
                height: sessionsListView.height / 4
                color: "black"
                anchors{
                    verticalCenter: parent.verticalCenter
                    left: sessionRect.right
                    leftMargin: 27
                }
                Text {
                   id: lapsText
                   anchors {
                       verticalCenter: parent.verticalCenter
                       left: parent.left
                       leftMargin: 60
                   }
                   color: "white"
                   font.pixelSize: 36
                   text: laps
                }
            }
            Rectangle{
                id: fastestLapRect
                width: 369
                height: sessionsListView.height / 4
                color: "black"
                anchors{
                    verticalCenter: parent.verticalCenter
                    left: lapsRect.right
                    leftMargin: 27
                }
                Text {
                   id: fastestLapText
                   anchors {
                       verticalCenter: parent.verticalCenter
                       left: parent.left
                       leftMargin: 100
                   }
                   color: "white"
                   font.pixelSize: 36
                   text: fastestLap
                }
            }
            MouseArea {
                anchors.fill: parent
                onPressed: {
                    dateRect.color = "white"
                    dateText.color = "black"

                    sessionRect.color = "white"
                    sessionText.color = "black"

                    lapsRect.color = "white"
                    lapsText.color = "black"

                    fastestLapRect.color = "white"
                    fastestLapText.color = "black"

                }
                onReleased: {
                  dateRect.color = "black"
                  dateText.color = "white"

                  sessionRect.color = "black"
                  sessionText.color = "white"

                  lapsRect.color = "black"
                  lapsText.color = "white"

                  fastestLapRect.color = "black"
                  fastestLapText.color = "white"

                }
                onClicked: {
                    console.log("Session Chosen Pressed")
                    myclass.addtoLapsList(rootObject, path)
                    mainLoader.source = "../LapsScreen/LapsScreen.qml"
                }
            }
        }
    }
}
