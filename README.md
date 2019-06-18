#TITLE:
EE450 USC (Computer Networking)

# Information:
All projects within are related to EE450 summer session 2019.
## Lab1:
Bellman Ford Shortest path
## Lab2:
DCHP TCP oriented unicast between single client and single dhcp server


## Phase1_Project:
In this phase, each department opens its input file (departmentA.txt, departmentB.txt or departmentC.txt) and opens a TCP connection with the admission office to send program names and the minimum GPA requirement of each program. It sends one packet per program that it has. This means that the department should know the static TCP port number of the admission office in advance. In other words you must hardcode the TCP port number of the admission office in the department code. Table 1 shows how static UDP and TCP port numbers should be defined for each entity in this project. Each department then uses one dynamically-assigned TCP port number (different for each department) and establishes one TCP connection to the admission office (see Requirement 1 of the project description). Thus, there are three different TCP connections to the admission office (one from each department).
As soon as the admission office receives the packets with program names and the minimum GPA requirements of the programs from all the departments, it stores locally the available programs in the system along with the department names who offer the programs. It is up to you to decide how you are going to store this information. It may be stored in a file or in the memory (e.g. through an array or a linked list of structs that you can define). Before you make this decision, you have to think of how a student sends their application later to the admission office. We expect that the admission office knows which programs are available, which department offer them and their minimum GPA requirements.
