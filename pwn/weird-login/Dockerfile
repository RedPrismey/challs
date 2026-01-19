FROM nsjail

RUN mkdir /app
WORKDIR /app

COPY data/ .

RUN chmod u+x /app/nsjail.sh && \
  chmod u+x /app/chall && \
  chmod u+x /app/ld-linux-x86-64.so.2 && \
  chmod u+x /app/libc.so.6 && \
  chmod u+r /app/flag.txt

EXPOSE 31337

ENTRYPOINT [ "/app/nsjail.sh" ]

CMD [ "/app/chall" ]
