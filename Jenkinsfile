pipeline {
    agent {
        docker { image 'localhost/crosscompile:latest' }
    }
    stages {
        stage('Test') {
            steps {
                buildprj -p Gnu MinGW Linux
            }
        }
    }
}
