FROM       ubuntu:latest

COPY . /

RUN apt-get update

RUN apt-get install -y gcc
RUN apt-get install -y socat

WORKDIR /src

RUN chmod +x ./start.sh

RUN ./exec.sh

ENTRYPOINT [ "./start.sh" ]