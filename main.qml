import QtQuick.Window 2.12
import QtQuick 2.12
import QtQuick.VirtualKeyboard 2.4

Window {
    id: window
    width: 1024
    height: 600
    visible: true
    title: qsTr("Racetrack Buddy")

    ListModel {
        id:lapslist
    }
    ListModel {
        id:reviewlist
    }
    ListModel{
        id:tracklist
    }
    function appendReview(newElement) {
           reviewlist.append(newElement)
       }
    function appendLaps(newElement) {
           lapslist.append(newElement)
       }
    function appendTracks(newElement) {
           tracklist.append(newElement)
       }
    Loader{
        id : mainLoader
        anchors.fill: parent
        source: "./UI/HomeScreen/HomeScreen.qml"
    }
}
