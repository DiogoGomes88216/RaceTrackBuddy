import QtQuick 2.0

Rectangle{
    id: trackSelectDialog
    signal destroyMe()

    anchors.fill: parent
    color: Qt.rgba(0,0,0,0)
    MouseArea{
        anchors.fill:parent
        onClicked:{
            trackSelectDialog.destroyMe()
            tracklist.clear()
        }
    }
    Rectangle{
        id:innerRectangle
        radius: 10
        width:parent.width / 2
        height: parent.height * 0.8
        anchors.centerIn: parent
        color: "#2C2C2C"
        border.color: "white"
        border.width: 3
    }
    ListView{
        id:trackListView
        anchors.fill: innerRectangle
        model: tracklist
        spacing: 13

        delegate: Rectangle{
            id: mainButton
            width: innerRectangle.width
            height: innerRectangle.height / 3
            color: "black"
            border.color: "White"
            border.width: 4
            radius: 10
            Text {
                anchors.centerIn: parent
                id: name
                color: "white"
                font.pixelSize: 42
                text: trackname
            }
            MouseArea {
                anchors.fill: parent
                onPressed: {
                    mainButton.color = "white"
                    name.color = "black"
                }
                onReleased: {
                    mainButton.color = "black"
                    name.color = "white"
                }
                onClicked: {
                    console.log("Track Selected")
                    myclass.setTrack(trackname,s1x_lat,s1x_lon,s1y_lat,s1y_lon,s2x_lat,s2x_lon,s2y_lat,s2y_lon,s3x_lat,s3x_lon,s3y_lat,s3y_lon)
                    tracklist.clear()
                    trackSelectDialog.destroyMe()
                }
           }
        }
    }
}
