FROM alpine:latest

LABEL version="1.0"
LABEL maintainer="mikhail.burov@skoltech.ru"

RUN apk --update add openssh-client git nodejs && rm -rf /var/cache/apk/* && npm install grunt-cli -g

RUN adduser -D -g "" node

USER node
WORKDIR /home/node
RUN git clone https://github.com/cubedro/eth-netstats

WORKDIR /home/node/eth-netstats
RUN npm install && grunt

ENTRYPOINT /bin/sh




