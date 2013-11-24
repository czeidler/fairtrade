Fair Trade 

Version 1: (2006) first BeOS version
Version 2: (Nov 2013) Adapt code to Haiku and a bit of cleanup to match the Haiku coding style. However, there are still style violations that I was not able to fix by now.

This program is designed for our local fair trade group (also see http://www.transfair.org/ (German link) for more info about the fair trade philosophy). The fair trade application supports sellers during the trade. Products can be scanned using a bar code scanner. All scanned products are showed in a shopping basket. For example, the seller scans a customer's product and the product appears in the shopping basket. If a customer wants to buy multiple products of the same kind, the product only has to be scanned once and the amount of products can be entered using the keypad.

Furthermore, it helps managing products on stock and provides an easy way to add new products. Sold products are automatically removed from the stock when completing a bargain. Product prices, descriptions, names and quantities can be edit as well. Old products can be disabled from the product view.

The design goal for the fair trade application was to make it as simple to use as possible. The application is supposed to run full screen and starts automatically when booting up. When leaving the application the computer is shut down automatically (optionally). Thus the user does not get in touch with the underlying OS.

To store the products a sqlite3 database is used. At every program exit the current list of products as well as a detailed list of all sales is automatically exported as CSV files. These files can be read by excel or OpenOffice to do some further statistics. 

The language of the Fair Trade application is German but it should be easy to translate it to different languages.

Key Navigation:
The fair trade application is supposed to be used with keyboard only. The reason for that is that we simply have not enough space for a mouse and we want to minimize the time setting up the laptop (what we have to do each time when opening the store). On our laptop we labeled the navigation keys with a special color code using small stickers. The current navigation options are clearly colored on the screen. This greatly helps people to use the application who are unfamiliar with the application or only use it occasionally. 

red: 	ESC         		(to abort a trade)
green: 	SPACE     			(to complete a trade)
blue: 	BACKSPACE  			(to remove a product from the shopping basket)
yellow:	TAB and arrow keys	(change fields / previous sales)

For manual bar code input the F2 key is used.

Bar Code Scanner:
Your Scanner should be configured in such a way that the ascii char 92 is used as post an prefix. If your scanner don't use this number you have to edit the value in mainwindow.cpp and recompile.

Known bugs:
When switching the desktop and switching back to Fair Trade the keyboard input stop working. You can switch again to another desktop and back and the problem is gone. Any suggestions?


Included is a small config app and a command line import CSV app:

- Config App
The config application gives some basic options to change the database location, the welcome texts, the CSV output dir and if to shutdown on exit.

- Import App
The import application allows to import a product list from a CSV file. For example, from one that has been exported by the FairTrade app.


Licence: Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de Distributed under the terms of the MIT License.
