docker build -t ubuntu:sshd .

docker run -d -p 2222:22 -v /run/media/edurbrito/Work/FEUP-3Y1S/RCOM/projects/rcom-projects/project-01/data:/root/data ubuntu:socat

ssh-keygen -R 172.17.0.3

ssh root@172.17.0.3

socat -d  -d  PTY,link=/dev/ttyS10,mode=777   PTY,link=/dev/ttyS11,mode=777