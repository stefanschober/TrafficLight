pipeline {
    agent {
        docker { image 'localhost/alpine:latest' }
    }
    stages {
        stage('Test') {
            steps {
                echo 'in container'
            }
        }
    }
}
