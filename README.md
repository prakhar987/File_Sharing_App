An Application Level ​File​ Sharing Protocol with support for download and upload for files and indexed searching. 
 
Features : 
1. The system has 2 clients (acting as servers simultaneously) listening to the communication channel for requests and waiting to share files (avoiding collisions) using an ​application layer protocol​ (like FTP/HTTP). 

2. Each client has the ability to do the following : 
-- Know the files present​ on each others machines in the designated shared folders. 
-- Download ​files​ from this shared folder. 
-- The system periodically checks for any changes made to the shared folders. 

3. File transfer incorporates ​MD5​ checksum​ to handle file transfer errors. 
 
USAGE ::
1. Run one server on one port and another server on another port (set PORT in one server.c file as 8000 &  another server.c as 8001 )
2. Now run two clients that connect to these ports (set PORT in one client.cpp file as 8001 & another as 8000)
3. Now run commands on clients (remember we have one machine (server+client) at 8000 and another machine at 8001)


List of Commands Supported : 
 
1. IndexGet ​flag (args) :
​
--can request the display of the shared files on the connected system. 
--the history of requests made by either clients should be maintained at each of the clients respectively. 
--the ​flag ​variable can be ​shortlist, longlist or regex​

		-shortlist : flag would mean that the client only wants to know the names of files between a specific set of timestamps. 
		eg : $> IndexGet ​shortlist <start​ time​stamp> <end​time​stamp> 
		Output : includes ‘name’, ‘size’, ‘timestamp’ and ‘type’ of the files between the start and end time stamps. 

		-longlist : flag means that client wants to know the entire listing of the shared folder/directory including ‘name’, ‘size’, ‘timestamp’ and ‘type’ of the files. 
 		eg : $> IndexGet ​longlist 
		Output : ​similar to above, but with complete file listing.

2. FileHash ​flag (args) : this command indicates that the client wants to check if any of the files on the other end have been changed. The flag variable can take two values, ​verify and ​checkall.​
 
		- verify : ​flag checks for the specific file name provided as command line argument and return its ‘checksum’ and ‘last​modified’ timestamp. 
		eg :  $> FileHash ​verify <filename> 
 		Output : ​checksum and last​ modified timestamp of the input file. 
		
		- checkall : ​flag performs what ‘verify’ does for all the files in the shared folder. 
		eg :  $> FileHash ​checkall 
	 	Output  : filename,  checksum  and  last​ modified  timestamp  of  all  the files in the shared directory. 

3. FileDownload ​flag (args): as  the  name  suggests,  would  be  used  to  download files  from  the  shared  folder of connected user to our shared folder. the  flag  variable  can  take  the  value  ​TCP or  ​
UDP  depending  on  the users request. If  a  socket  is  not  available,  it  is  created  and  both  clients  use  this socket for file transfer.
		
		-- TCP 
		eg: $> FileDownload TCP <filename> 
		Output  :  ​contains  the  filename,  filesize,  last​ modified  timestamp  and the MD5​hash of the requested file.  
