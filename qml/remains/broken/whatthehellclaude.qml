import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal
import Qt.labs.qmlmodels

ApplicationWindow {
    id: window
    width: 800
    height: 600
    visible: true
    title: "QML TreeView Example"
    
    // Enable Universal theme to respect system theme
    Universal.theme: Universal.System
    Universal.accent: Universal.Blue
    
    // Tree model with hardcoded data
    TreeModel {
        id: treeModel
        
        TreeModelNode {
            display: "Documents"
            expanded: true
            TreeModelNode {
                display: "Projects"
                expanded: true
                TreeModelNode {
                    display: "QML App"
                    TreeModelNode {
                        display: "main.qml"
                    }
                    TreeModelNode {
                        display: "components"
                        TreeModelNode {
                            display: "Button.qml"
                        }
                        TreeModelNode {
                            display: "Header.qml"
                        }
                    }
                }
                TreeModelNode {
                    display: "Flutter App"
                    TreeModelNode {
                        display: "lib"
                        TreeModelNode {
                            display: "main.dart"
                        }
                        TreeModelNode {
                            display: "widgets"
                            TreeModelNode {
                                display: "home_page.dart"
                            }
                        }
                    }
                    TreeModelNode {
                        display: "pubspec.yaml"
                    }
                }
            }
            TreeModelNode {
                display: "Personal"
                TreeModelNode {
                    display: "Photos"
                    TreeModelNode {
                        display: "Vacation 2024"
                        TreeModelNode {
                            display: "IMG_001.jpg"
                        }
                        TreeModelNode {
                            display: "IMG_002.jpg"
                        }
                    }
                }
                TreeModelNode {
                    display: "Notes.txt"
                }
            }
        }
        
        TreeModelNode {
            display: "Downloads"
            TreeModelNode {
                display: "Software"
                TreeModelNode {
                    display: "Qt6_installer.exe"
                }
                TreeModelNode {
                    display: "VSCode_setup.exe"
                }
            }
            TreeModelNode {
                display: "Archives"
                TreeModelNode {
                    display: "backup_2024.zip"
                }
            }
        }
        
        TreeModelNode {
            display: "Music"
            expanded: true
            TreeModelNode {
                display: "Rock"
                TreeModelNode {
                    display: "The Beatles"
                    TreeModelNode {
                        display: "Abbey Road"
                        TreeModelNode {
                            display: "Come Together.mp3"
                        }
                        TreeModelNode {
                            display: "Something.mp3"
                        }
                    }
                }
                TreeModelNode {
                    display: "Led Zeppelin"
                    TreeModelNode {
                        display: "IV"
                        TreeModelNode {
                            display: "Stairway to Heaven.mp3"
                        }
                    }
                }
            }
            TreeModelNode {
                display: "Electronic"
                TreeModelNode {
                    display: "Daft Punk"
                    TreeModelNode {
                        display: "Random Access Memories"
                        TreeModelNode {
                            display: "Get Lucky.mp3"
                        }
                    }
                }
            }
        }
    }
    
    // Main content
    ScrollView {
        anchors.fill: parent
        anchors.margins: 10
        
        TreeView {
            id: treeView
            anchors.fill: parent
            model: treeModel
            rootIndex: treeModel.index(0, 0)
            
            // Custom delegate for tree items
            delegate: TreeViewDelegate {
                id: treeDelegate
                
                Rectangle {
                    anchors.fill: parent
                    color: treeDelegate.current ? Universal.accent : 
                           (treeDelegate.hovered ? Universal.baseLowColor : "transparent")
                    
                    Row {
                        anchors.left: parent.left
                        anchors.leftMargin: 8 + (treeDelegate.depth * 20)
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 4
                        
                        // Expand/collapse indicator
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: treeDelegate.hasChildren ? 
                                  (treeDelegate.expanded ? "▼" : "▶") : "  "
                            color: Universal.foreground
                            font.pixelSize: 12
                            width: 16
                        }
                        
                        // Icon based on item type
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: {
                                var name = model.display.toLowerCase()
                                if (treeDelegate.hasChildren) {
                                    return "📁"
                                } else if (name.endsWith('.qml') || name.endsWith('.dart')) {
                                    return "📄"
                                } else if (name.endsWith('.jpg') || name.endsWith('.png')) {
                                    return "🖼️"
                                } else if (name.endsWith('.mp3') || name.endsWith('.wav')) {
                                    return "🎵"
                                } else if (name.endsWith('.zip') || name.endsWith('.rar')) {
                                    return "📦"
                                } else if (name.endsWith('.exe')) {
                                    return "⚙️"
                                } else {
                                    return "📄"
                                }
                            }
                            font.pixelSize: 16
                        }
                        
                        // Item text
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: model.display
                            color: treeDelegate.current ? Universal.chromeWhiteColor : Universal.foreground
                            font.pixelSize: 14
                        }
                    }
                    
                    // Mouse handling
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            treeView.toggleExpanded(row)
                            treeDelegate.current = true
                        }
                        onDoubleClicked: {
                            if (!treeDelegate.hasChildren) {
                                console.log("Opening file:", model.display)
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Status bar
    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 30
        color: Universal.chromeMediumColor
        
        Text {
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            text: "TreeView Example - Double-click files to open"
            color: Universal.baseHighColor
            font.pixelSize: 12
        }
    }
}
