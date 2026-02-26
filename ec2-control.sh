#!/bin/bash
set -euo pipefail

INSTANCE_ID="i-0517faa0f621adf01"
AWS_REGION="mx-central-1"   # change if needed
AWS_PROFILE="default"   # change if needed

aws_cmd() {
  aws --region "$AWS_REGION" --profile "$AWS_PROFILE" "$@"
}

show_status() {
  aws_cmd ec2 describe-instances --instance-ids "$INSTANCE_ID" \
    --query 'Reservations[0].Instances[0].[InstanceId,State.Name,PublicIpAddress,InstanceType]' \
    --output table
}

case "${1:-}" in
  start)
    echo "Starting EC2 instance: $INSTANCE_ID"
    aws_cmd ec2 start-instances --instance-ids "$INSTANCE_ID" >/dev/null
    echo "Waiting for instance to be running..."
    aws_cmd ec2 wait instance-running --instance-ids "$INSTANCE_ID"
    echo "Instance is now running."
    show_status
    ;;
  stop)
    echo "Stopping EC2 instance: $INSTANCE_ID"
    aws_cmd ec2 stop-instances --instance-ids "$INSTANCE_ID" >/dev/null
    echo "Waiting for instance to stop..."
    aws_cmd ec2 wait instance-stopped --instance-ids "$INSTANCE_ID"
    echo "Instance is now stopped."
    show_status
    ;;
  status)
    echo "Status of EC2 instance: $INSTANCE_ID"
    show_status
    ;;
  *)
    echo "Usage: $0 {start|stop|status}"
    exit 1
    ;;
esac

