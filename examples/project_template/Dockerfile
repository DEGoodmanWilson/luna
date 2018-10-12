FROM degoodmanwilson/luna:latest

MAINTAINER D.E. Goodman-Wilson

ENV PORT 8080
EXPOSE 8080
WORKDIR /app
ADD . /app
RUN sudo chown -R conan . && \
	conan --version && \
	conan install . && \
	conan profile show default && \
	cmake . && \
	cmake --build .
CMD ["./bin/awesomesauce"]
