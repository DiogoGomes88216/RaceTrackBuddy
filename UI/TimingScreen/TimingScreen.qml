import QtQuick 2.0

Item {
    id:timingScreen
    Image{
        source: "TimingScreen.png"
        id: timingBackground
        anchors.fill: parent
    }
    Image{
        id: stopSign
        x: 42
        y: 479
        width: 100
        height: 100
        source: "qrc:/UI/Assets/stop-sign.png"

        MouseArea{
            anchors.fill : parent
            onClicked:{
                console.log("Stop Session Buttton Pressed")
                myclass.stopSession()
                mainLoader.source = "../HomeScreen/HomeScreen.qml"
            }
        }
    }
    Rectangle{
        id: deltaRectangle
        x: 0
        y: 175
        width: 1024
        height: 262
        color:{
            if(myclass.lapDelta[0] === "-")
                return "green"
            else
                return "red"
        }
    }
    Text{
        id: deltaLabel
        x: 450
        y: 175
        color: "white"
        font.pixelSize: 48
        text: "Delta"
    }
    Text{
        id: lapDelta
        x: 50
        y: 224
        color: "white"
        font.pixelSize: 175
        text: myclass.lapDelta
    }
    Text{
        id: lastLap
        x: 93
        y: 66
        color: "white"
        font.pixelSize: 72
        text: myclass.lastLap
    }
    Text{
        id: bestLap
        x: 606
        y: 66
        color: "purple"
        font.pixelSize: 72
        text: myclass.bestLap
    }
    Text{
        id: totalTime
        x: 560
        y: 490
        color: "white"
        font.pixelSize: 96
        text: myclass.totalTime
    }
    Text{
        id: lapNumber
        x: 315
        y: 490
        color: "white"
        font.pixelSize: 96
        text: myclass.lapNumber
    }
}
