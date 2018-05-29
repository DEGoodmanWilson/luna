FROM degoodmanwilson/luna:5.0.2

MAINTAINER D.E. Goodman-Wilson

ENV PORT 8080
EXPOSE 8080
WORKDIR /app
ADD . /app
RUN sudo chown -R conan .
RUN conan --version
RUN conan install .
RUN conan profile show default
RUN cmake .
RUN cmake --build .
CMD ["./bin/awesomesauce"]
