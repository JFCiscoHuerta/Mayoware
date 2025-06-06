# Debian base image
FROM debian:latest

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \  
    gcc \
    mingw-w64 \
    libcriterion-dev \
    nano \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy source files
COPY c2.c agent.c test_c2.c c2.h .

# Compile
RUN gcc -o c2 c2.c

# Expose the server's listening port
EXPOSE 50004

# RUN
CMD ["/app/c2"]