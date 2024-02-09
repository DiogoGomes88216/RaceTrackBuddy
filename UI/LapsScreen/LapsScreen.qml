import QtQuick 2.0

Item {
    Image{
        source: "LapsScreen.png"
        id: lapsScreen
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
                lapslist.clear()
                mainLoader.source = "../ReviewScreen/ReviewScreen.qml"
            }
        }
    }
    Image{
        id: homeIcon
        x: 238
        y: 20
        width: 66
        height: 62
        source: "qrc:/UI/Assets/Home.png"

        MouseArea{
            anchors.fill : parent
            onClicked:{
                console.log("Home Buttton Pressed")
                lapslist.clear()
                reviewlist.clear()
                mainLoader.source = "../HomeScreen/HomeScreen.qml"
            }
        }
    }
    Image{
        id: videoIcon
        x: 683
        y: 8
        width: 238
        height: 80
        source: "qrc:/UI/Assets/VideoButton.png"

        MouseArea{
            anchors.fill : parent
            onClicked:{
                console.log("Video Button Pressed")
                mainLoader.source = "../VideoScreen/VideoScreen.qml"
            }
        }
    }

    ListView {
        id: lapsListView
        width: 1006 ; height: 417
        x: 9
        y: 169
        spacing: 9
        model: lapslist
        delegate:
            Rectangle{
                id: mainButton
                width: lapsListView.width
                height: 50
                color: "transparent"
                    Rectangle{
                        id: lapRect
                        width: 107
                        height: parent.height
                        color: setColor
                        anchors {
                            left: parent.left
                        }
                        Text {
                           id: lapText
                           anchors {
                               verticalCenter: parent.verticalCenter
                               left: parent.left
                               leftMargin: 15
                           }
                           color: "white"
                           font.pixelSize: 36
                           text: lap
                        }
                    }
                    Rectangle{
                        id: lapTimeRect
                        width: 258
                        height: parent.height
                        color: setColor
                        anchors {
                            left: lapRect.right
                            leftMargin: 13
                        }
                        Text {
                           id: lapTimeText
                           anchors {
                               verticalCenter: parent.verticalCenter
                               left: parent.left
                               leftMargin: 40
                           }
                           color: "white"
                           font.pixelSize: 36
                           text: lapTime
                        }
                    }
                    Rectangle{
                        id: s1Rect
                        width: 144
                        height: parent.height
                        color: setColor
                        anchors {
                            left: lapTimeRect.right
                            leftMargin: 13
                        }
                        Text {
                           id: s1Text
                           anchors {
                               verticalCenter: parent.verticalCenter
                               left: parent.left
                               leftMargin: 2
                           }
                           color: "white"
                           font.pixelSize: 28
                           text: s1
                        }
                    }
                    Rectangle{
                        id: s2Rect
                        width: 144
                        height: parent.height
                        color: setColor
                        anchors {
                            left: s1Rect.right
                            leftMargin: 13
                        }
                        Text {
                           id: s2Text
                           anchors {
                               verticalCenter: parent.verticalCenter
                               left: parent.left
                               leftMargin: 2
                           }
                           color: "white"
                           font.pixelSize: 28
                           text: s2
                        }
                    }
                    Rectangle{
                        id: s3Rect
                        width: 144
                        height: parent.height
                        color: setColor
                        anchors {
                            left: s2Rect.right
                            leftMargin: 13
                        }
                        Text {
                           id: s3Text
                           anchors {
                               verticalCenter: parent.verticalCenter
                               left: parent.left
                               leftMargin: 2
                           }
                           color: "white"
                           font.pixelSize: 28
                           text: s3
                        }
                    }
                    Rectangle{
                        id: gapRect
                        width: 142
                        height: parent.height
                        color: setColor
                        anchors {
                            left: s3Rect.right
                            leftMargin: 13
                        }
                        Text {
                           id: gapText
                           anchors {
                               verticalCenter: parent.verticalCenter
                               left: parent.left
                               leftMargin: 2
                           }
                           color: "white"
                           font.pixelSize: 28
                           text: gap
                        }
                    }
            }
    }
}
