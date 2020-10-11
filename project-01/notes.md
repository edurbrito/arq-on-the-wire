>Create Docker Image
docker build -t ubuntu:socat .

docker run -d -p 2222:22 -v /media/pedro/506AB3243AE91363/FEUP/3Ano/RCOM/Trabalho/RCOM/project-01/data:/root/data ubuntu:socat

>Create new SSH Keys
ssh-keygen -R 172.17.0.3

>Create Connection
ssh root@172.17.0.3

>Socat
socat -d  -d  PTY,link=/dev/ttyS10,mode=777   PTY,link=/dev/ttyS11,mode=777 &

> Start Docker Service if not Available
sudo systemctl start docker 