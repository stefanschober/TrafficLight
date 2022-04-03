pipeline {
    agent {
        docker { image 'localhost/alpine:latest' }
    }
    stages {
        stage('Test') {
            steps {
                sh 'buildprj -p Gnu'
                sh 'buildprj -p MinGW'
                sh 'buildprj -p Raspi'
            }
        }
    }
}
