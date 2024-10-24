String node_label
String gcp_project

if (params.ENV == "NPE") {
    node_label = "n"
    gcp_project = "your-npe-project-id"
} else if (params.ENV == "UAT") {
    node_label = "uat-node"
    gcp_project = "your-uat-project-id"
} else if (params.ENV == "PROD") {
    node_label = "prod-node"
    gcp_project = "your-prod-project-id"
}

pipeline {
  agent none
  parameters {
    choice(choices: ['NPE', 'UAT', 'PROD'], description: 'Which Environment?', name: 'ENV')
    string(name: 'SourceBucketPath', description: 'Source GCS bucket path (gs://bucket-name/path)', trim: true)
    string(name: 'DestinationBucketPath', description: 'Destination GCS bucket path (gs://bucket-name/path)', trim: true)
  }
  stages {
    stage('Request Approval') {
      steps {
        script {
          def approvers = '' // Define approvers based on the environment
          if (params.ENV == 'NPE') {
            approvers = 'npe-user1, npe-user2'
          } else if (params.ENV == 'UAT') {
            approvers = 'uat-user1, uat-user2'
          } else if (params.ENV == 'PROD') {
            approvers = 'prod-user1, prod-user2'
          }

          input message: "Request approval for ${params.ENV} environment", 
                ok: 'Proceed with Copy', 
                submitter: approvers, 
                parameters: [
                  string(name: 'approver_comment', description: 'Please enter your comment before approving')
                ]
        }
      }
    }
    
    stage('Copy files') {
      agent {
        label "$node_label"
      }
      steps {
        script {
          sh """
            set -e
            echo "Listing configuration"
            gcloud config set project $gcp_project
            gcloud config list
            echo "Copying files from $params.SourceBucketPath to $params.DestinationBucketPath in GCS"
            gsutil -m cp $params.SourceBucketPath $params.DestinationBucketPath
          """
        }
      }
    }
  }
}
