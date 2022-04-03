pipeline {
    agent {
        docker {
            image 'localhost/crosscompile:latest'
            args '--entrypoint='
        }
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
