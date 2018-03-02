FROM alpine:latest

LABEL version="1.0"
LABEL maintainer="mikhail.burov@skoltech.ru"

RUN apk update && apk upgrade
RUN apk add geth

RUN adduser -D -g "" node

COPY eth_common /home/node/eth_common
RUN chown -R node:node /home/node/eth_common
USER node
WORKDIR /home/node

RUN geth --password eth_common/password account new
RUN geth init eth_common/genesis.json

ENTRYPOINT /bin/sh


