import QtQuick 2.0
import QtMultimedia 5.15

Item {
    Image{
        source: "VideoScreen.png"
        id: videoScreen
        anchors.fill: parent
    }
    Image{
        id: backIcon
        x: 85
        y: 9
        width: 60
        height: 60
        source: "qrc:/UI/Assets/back_arrow.png"

        MouseArea{
            anchors.fill : parent
            onClicked:{
                console.log("Back Buttton Pressed")
                mainLoader.source = "../LapsScreen/LapsScreen.qml"
            }
        }
    }
    Image{
        id: homeIcon
        x: 246
        y: 11
        width: 50
        height: 50
        source: "qrc:/UI/Assets/Home.png"

        MouseArea{
            anchors.fill : parent
            onClicked:{
                console.log("Home Buttton Pressed")
                mainLoader.source = "../HomeScreen/HomeScreen.qml"
            }
        }
    }
    MediaPlayer {
           id: player
           source: "/etc/test.avi"
       }

    VideoOutput {
           id: videoOutput
           source: player
           anchors.fill: parent
       }
    MouseArea {
           id: playArea
           anchors.fill: parent
           onPressed: player.play();
       }
}
