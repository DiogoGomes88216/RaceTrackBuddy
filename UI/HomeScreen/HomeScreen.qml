import QtQuick 2.0

Item {
    id:homeScreen
    property var trackSelectDialogHolder: null

    function createTrackSelectDialog() {
            if ( trackSelectDialogHolder === null ) {
                var component = Qt.createComponent( "TrackSelectDialog.qml" )
               trackSelectDialogHolder = component.createObject( homeScreen, {"x":0,"y":0})
                if ( trackSelectDialogHolder ) {
                    trackSelectDialogHolder.anchors.fill = homeScreen
                    trackSelectDialogHolder.destroyMe.connect( destroyTrackSelectDialog )
                }
            }
        }
    function destroyTrackSelectDialog() {
        if ( trackSelectDialogHolder !== null ) {
            trackSelectDialogHolder.destroy()
            trackSelectDialogHolder = null
        }
    }

    Image{
        source: "HomeScreen.png"
        id:mainBackground
        anchors.fill: parent
    }
    Image{
        id:gpsFix
        x: 405
        y: 4
        width: 40
        height: 40
        source:
        {
            if(myclass.gpsFix)
              return "qrc:/UI/Assets/gps_fixed.png"
            else
              return "qrc:/UI/Assets/gps_not_fixed.png"
        }
    }
    Text{
        id:currentTime
        x: 470
        y: 4
        color: "white"
        font.pixelSize: 36
        text: myclass.currentTime
    }
    MouseArea {
        id : startButton
        x : 180
        y : 131
        width : 289
        height : 339
        onClicked :
        {
            console.log("Start Buttton Pressed")
            if(myclass.gpsFix && myclass.currTrack !== "Select a Track!")
            {
                myclass.startSession()
                mainLoader.source = "../TimingScreen/TimingScreen.qml"
            }
        }
    }
    MouseArea {
        id : reviewButton
        x : 555
        y : 131
        width : 289
        height : 339
        onClicked :
        {
            console.log("Review Buttton Pressed")
            myclass.addToReviewList(rootObject)
            mainLoader.source = "../ReviewScreen/ReviewScreen.qml"
        }
    }
    Image{
        id:trackSelect
        x: 36
        y: 25
        width: 100
        height: 138
        source: "qrc:/UI/Assets/Track.png"

        MouseArea{
            anchors.fill : parent
            onClicked:{
                console.log("Track Select button Press")
                myclass.addtoTrackList(rootObject)
                createTrackSelectDialog()
            }
        }
    }
    Text{
        id:currentTrack
        x: 330
        y: 536
        color: "white"
        font.pixelSize: 48
        text: myclass.currTrack
    }
    Text{
        id:demoText
        x: 905
        y: 8
        color: "white"
        font.pixelSize: 18
        text: "Demo Mode"
    }
    Image{
        id:toggleDemo
        x: 905
        y: 8
        width: 100
        height: 100
        source:
        {
            if(myclass.demoMode)
              return "qrc:/UI/Assets/toggle-on.png"
            else
              return "qrc:/UI/Assets/toggle-off.png"
        }

        MouseArea{
            anchors.fill : parent
            onClicked:{
                console.log("Demo Mode Toggle")
                if(myclass.demoMode === false)
                    myclass.setDemoMode(true);
                else
                    myclass.setDemoMode(false);
            }
        }
    }
}
