import QtQuick 2.5
import QtQuick.Particles 2.0

Item {
    required property string modelData
    required property string color
    // property string storedColor: modelData
    // property string storedColor: "crimson"

    // idk why you need two variables here

    id: base

    // implicitWidth: 200
    height: 50
    width: 50
    

    Rectangle {
        anchors.fill: parent
        opacity: 0.2
        color: base.color

        // bottomMargin: 100
    }

    ParticleSystem {
        id: particleSystem
    }

    Emitter {
        system: particleSystem
        id: emitter
        anchors {
            // bottom: parent.bottom
            // bottomMargin: 100
            horizontalCenter: parent.horizontalCenter
        }

        width: 80; height: 80
        emitRate: 10
        lifeSpan: 2000
        lifeSpanVariation: 500
        size: 40
        endSize: 60
        // Tracer { color: 'green' }

        velocity: AngleDirection {
            angle: -90
            angleVariation: 20
            magnitude: 50
        }
        acceleration: AngleDirection {
            angle: -90
            magnitude: 120
        }
    }

    ImageParticle {
        // source: "assets/particle.png"
        // source: "tiny-sphere-gradient.png"
        source: "qrc:///particleresources/glowdot.png"
        // source: "qrc:///particleresources/fuzzydot.png"
        color: base.color
        colorVariation: 0.1
        system: particleSystem
    }
}
