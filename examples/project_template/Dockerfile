FROM degoodmanwilson/luna-node

MAINTAINER D.E. Goodman-Wilson

ENV PORT 80
EXPOSE 80
WORKDIR /app
ADD . /app
RUN conan --version
RUN conan install .
RUN cmake .
RUN cmake --build .
CMD ["./bin/awesomesauce"]
