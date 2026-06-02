MPI Array Sum Dashboard
An interactive web dashboard for computing the sum of large arrays using MPI (Message Passing Interface). This project demonstrates parallel processing concepts by distributing array summation across multiple compute nodes, with real-time visualization of the Master-Worker communication and data aggregation.

Live Demo: https://pdc-1.onrender.com

Note: The application is hosted on a free tier. Please allow up to 30 seconds for the server to wake up if it has been inactive.

Architecture and Workflow
The application uses a client-server architecture combined with a Master-Worker parallel computing model:

The web frontend allows the user to configure the number of MPI processes.
The Flask backend receives the request and spawns an MPI subprocess.
The Master process (Rank 0) generates an array of 100 integers, partitions the data, and distributes chunks to the Worker processes.
Each Worker process computes a partial sum of its assigned chunk and transmits it back to the Master.
The Master aggregates the partial sums to calculate the final total.
The backend parses the output and returns the logs, metrics, and final result to the dashboard for visualization.
Key Features
Dynamic process configuration (2 to 8 nodes)
Real-time execution log displaying Master and Worker communication
Visual progress bars tracking individual worker activity
Automated metrics calculation (execution time, throughput)
Responsive dark-themed user interface
Technical Specifications
Data Partitioning: The Master process divides the array size evenly among workers, handling remainders to ensure balanced workloads.
Communication Model: Utilizes MPI Send and Receive protocols for chunk distribution and partial sum collection.
Result Aggregation: The Master computes the global sum by adding all received partial sums.
Tech Stack
Parallel Computing: MPI (Message Passing Interface)
Backend: C, Python, Flask, Flask-CORS
Frontend: HTML5, CSS3, JavaScript, Chart.js
DevOps: Docker, Git, GitHub Codespaces, Render.com


MPICH or OpenMPI installed
Python 3.x installed
