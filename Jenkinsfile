pipeline {
    agent {
        docker { image 'localhost/archlinux:latest' }
    }
    stages {
        stage('Test') {
            steps {
                sh '--version'
                echo 'in container'
            }
        }
    }
}
