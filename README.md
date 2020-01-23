# IR-Remote-PC-Interface
The infrared(IR) remote PC interface allows user to send information from an IR remote to a receiver within 3 meters, and displays it on a terminal session opened on a PC.

# How the device works
The user presses a button on the remote within 3 metres of the receiver and the
remote sends a signal to the receiver. The signal received contains the 8-bit
hexadecimal address and 8-bit data code corresponding to the button pressed. The
codes will then be displayed in a terminal session opened on the PC. The format in
which the codes are displayed is as follows: One code is to be displayed on each line in
the following format:

Address: AA, Data: DD

where AA are the two hexadecimal digits corresponding to the address code of
the IR remote message, and DD is the same for the data.
If the code is not a valid code, an invalid message is displayed in the terminal
session and the interface will be ready to receive the next code. If the user holds down
the button, a repeat code is sent and the same hexadecimal address and data code as
the initial code when the button is pressed is displayed in the terminal session, until the
user releases the button or an invalid code is received.

# Some background
This is the 4th project in our ECE 312: Embedded Systems Design course that my teammate, Simon, and I had to do. Notably, the hardest project in the course. With some luck, we managed to complete it and have it work. This is evident in the following lines in the main.c file:

/*print address code and data code to terminal session*/
	printf("Address: %x  ,  Data: %x%s\r\n", buff[0] , buff[2],"  "); //the string of exactly 2 spaces makes it work. No idea why, but it works	
	for (int i = 0 ; i < 4 ; i++)	//for loop to clear buff[]
	{
		buff[i] = 0;
	}
}
/*end*/

As commented, I have no idea why adding exactly 2 spaces makes this line of code work, and neither did our TA. I don't remember why I decided to add spaces at the time, but it was by trial and error that we got this code to work.

# My contributions
Lots of hours spent debugging the code which ended up with me finding the solution to the problem (i.e. adding a string of spaces to the line of code as mentioned above).

# What I learned
The importance of a good flow chart, how to interface an IR remote with the PC and communicate between the two, and that sometimes coding by brute force (with a little help from lady luck) works too.

