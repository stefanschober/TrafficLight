pipeline {
    agent {
        docker { image 'localhost/crosscompile:latest' }
    }
    stages {
        stage('Test') {
            steps {
                sh 'buildprj -p Gnu MinGW'
            }
        }
    }
}
