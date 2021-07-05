# Multi-Threaded-Ticket-Reservation-System

The objective of the program is to design a system, which makes reservations on
behalf of the clients. There are 3 teller threads keep working to handle the reservation
for client threads. Each teller thread handles some tasks and whenever they finish their job,
they continue with the other clients. If multiple teller threads are available, it is sorted as
A, B, C in order of dealing tasks.
