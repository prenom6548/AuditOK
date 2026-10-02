import QtQuick 2.0
import org.ukui.positioner 1.0

Item {
    width: 600
    height: 600
    
    // Test 1: Basic UkuiFlow with LeftToRight flow
    UkuiFlow {
        id: flow1
        width: 200
        height: 200
        flow: UkuiFlow.LeftToRight
        spacing: 5
        
        Rectangle { width: 50; height: 50; color: "red" }
        Rectangle { width: 50; height: 50; color: "green" }
        Rectangle { width: 50; height: 50; color: "blue" }
        Rectangle { width: 50; height: 50; color: "yellow" }
    }
    
    // Test 2: UkuiFlow with TopToBottom flow
    UkuiFlow {
        id: flow2
        x: 210
        width: 100
        height: 200
        flow: UkuiFlow.TopToBottom
        spacing: 10
        
        Rectangle { width: 80; height: 40; color: "red" }
        Rectangle { width: 80; height: 40; color: "green" }
        Rectangle { width: 80; height: 40; color: "blue" }
    }
    
    // Test 3: UkuiFlow with RightToLeft layout
    UkuiFlow {
        id: flow3
        y: 210
        width: 200
        height: 100
        layoutDirection: Qt.RightToLeft
        spacing: 5
        
        Rectangle { width: 40; height: 40; color: "red" }
        Rectangle { width: 40; height: 40; color: "green" }
        Rectangle { width: 40; height: 40; color: "blue" }
    }
    
    // Test 4: UkuiFlow with lineHorizontalCenterAlignment (LeftToRight)
    UkuiFlow {
        id: flow4
        x: 210
        y: 210
        width: 150
        height: 150
        lineHorizontalCenterAlignment: true
        spacing: 5
        
        Rectangle { width: 40; height: 30; color: "red" }
        Rectangle { width: 40; height: 30; color: "green" }
        Rectangle { width: 40; height: 30; color: "blue" }
        Rectangle { width: 40; height: 30; color: "yellow" }
        Rectangle { width: 40; height: 30; color: "purple" }
    }
    
    // Test 4b: UkuiFlow with lineHorizontalCenterAlignment and RightToLeft
    UkuiFlow {
        id: flow4b
        x: 370
        y: 210
        width: 150
        height: 150
        lineHorizontalCenterAlignment: true
        layoutDirection: Qt.RightToLeft
        spacing: 5
        
        Rectangle { width: 40; height: 30; color: "red" }
        Rectangle { width: 40; height: 30; color: "green" }
        Rectangle { width: 40; height: 30; color: "blue" }
        Rectangle { width: 40; height: 30; color: "yellow" }
        Rectangle { width: 40; height: 30; color: "purple" }
    }
    
    // Test 4c: UkuiFlow with lineHorizontalCenterAlignment - multiple rows
    UkuiFlow {
        id: flow4c
        x: 530
        y: 210
        width: 100
        height: 150
        lineHorizontalCenterAlignment: true
        spacing: 5
        
        Rectangle { width: 40; height: 30; color: "red" }
        Rectangle { width: 40; height: 30; color: "green" }
        Rectangle { width: 40; height: 30; color: "blue" }
        Rectangle { width: 40; height: 30; color: "yellow" }
        Rectangle { width: 40; height: 30; color: "purple" }
        Rectangle { width: 40; height: 30; color: "orange" }
        Rectangle { width: 40; height: 30; color: "cyan" }
        Rectangle { width: 40; height: 30; color: "magenta" }
    }
    
    // Test 5: UkuiFlow with padding
    UkuiFlow {
        id: flow5
        x: 100
        y: 320
        width: 200
        height: 80
        topPadding: 10
        bottomPadding: 10
        leftPadding: 5
        rightPadding: 5
        spacing: 5
        
        Rectangle { width: 30; height: 30; color: "red" }
        Rectangle { width: 30; height: 30; color: "green" }
        Rectangle { width: 30; height: 30; color: "blue" }
    }
    
    // Test 6: UkuiFlow with add transition
    UkuiFlow {
        id: flow6
        x: 400
        width: 150
        height: 200
        flow: UkuiFlow.LeftToRight
        spacing: 5
        
        add: Transition {
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 300 }
        }
        
        Rectangle { width: 40; height: 40; color: "red" }
        Rectangle { width: 40; height: 40; color: "green" }
        Rectangle { width: 40; height: 40; color: "blue" }
    }
    
    // Test 7: UkuiFlow with move transition
    UkuiFlow {
        id: flow7
        x: 400
        y: 210
        width: 150
        height: 200
        flow: UkuiFlow.LeftToRight
        spacing: 5
        
        move: Transition {
            NumberAnimation { property: "x"; duration: 300 }
        }
        
        Rectangle { width: 40; height: 40; color: "red" }
        Rectangle { width: 40; height: 40; color: "green" }
    }
    
    // Test 8: UkuiFlow with populate transition
    UkuiFlow {
        id: flow8
        x: 400
        y: 420
        width: 150
        height: 150
        flow: UkuiFlow.LeftToRight
        spacing: 5
        
        populate: Transition {
            NumberAnimation { property: "scale"; from: 0; to: 1; duration: 300 }
        }
        
        Rectangle { width: 40; height: 40; color: "red" }
        Rectangle { width: 40; height: 40; color: "green" }
    }
    
    // Test 9: UkuiFlow with all paddings
    UkuiFlow {
        id: flow9
        y: 400
        width: 180
        height: 180
        topPadding: 15
        bottomPadding: 15
        leftPadding: 10
        rightPadding: 10
        spacing: 8
        
        Rectangle { width: 30; height: 30; color: "red" }
        Rectangle { width: 30; height: 30; color: "green" }
        Rectangle { width: 30; height: 30; color: "blue" }
    }
    
    // Test 10: UkuiFlow with multiple rows (TopToBottom)
    UkuiFlow {
        id: flow10
        x: 210
        y: 370
        width: 180
        height: 200
        flow: UkuiFlow.TopToBottom
        spacing: 5
        
        Rectangle { width: 50; height: 30; color: "red" }
        Rectangle { width: 50; height: 30; color: "green" }
        Rectangle { width: 50; height: 30; color: "blue" }
        Rectangle { width: 50; height: 30; color: "yellow" }
        Rectangle { width: 50; height: 30; color: "purple" }
        Rectangle { width: 50; height: 30; color: "orange" }
    }
}
