# Web-Crawling-System
This is a simple crawling system that allows HTML downloading as well as any content (images, archives, etc.) marked with &lt;a href=".

The project consists of two applications, a client and a server. The client interacts only with the server. It acts like a worker, actually. The server is started up
with some command line parameters, like: recursion mode, everything mode, and a link to a web page that it has to download. The server has to deliver that initial
link that it received from the user to a connected client. The client connects to that specific web-server, it downloads the page (sends a GET request for the HTML
page) and then, if the recursive mode is enabled, it scans the entire HTML page and collects the links that are relative to the root. Those links are then transmitted
back to server, along with the downloaded HTML page. The server then put those links in a list and distributes them equally to the connected clients.


