pipeline {
    agent {
        docker { image 'docker.io/library/archlinux:latest' }
    }
    stages {
        stage('Test') {
            steps {
                sh 'bash --version'
                echo 'in container'
            }
        }
    }
}
