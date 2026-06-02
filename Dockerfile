FROM python:3.10-slim

# Install MPI (mpich) and C compiler
RUN apt-get update && apt-get install -y mpich gcc && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy requirements and install Python dependencies
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy the rest of the application code
COPY . .

# Compile the C MPI program
RUN mpicc -Wall -O2 -o mapreduce mapreduce.c

# Expose the port
EXPOSE 5000

# Command to run the application
CMD ["python", "backend.py"]
