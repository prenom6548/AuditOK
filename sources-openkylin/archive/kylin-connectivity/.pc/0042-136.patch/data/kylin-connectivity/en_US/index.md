# kylin-connectivity
## Summary

kylin-connectivity is a tool that can help users quickly discover and connect kylinos and Android devices under the same LAN, and implements the rapid discovery, device connection, file sharing, screen sharing and reverse control between kylinos and between kylinos and Android devices.


## Attended mode

kylin-connectivity supports a variety of connection modes, open the kylin-connectivity, click the immediate connection, enter the connection interface, you can choose a variety of connection modes, including device discovery, scanning code connection, usb connection, and device code connection.

![Figure 1 Home page-big](image/home-page.png)

#### Device Discovery

Opening the device, you can find all kylinos with kylin-connectivity open under the current LAN and Android devices with kylin-connectivity phone app open.

![Figure 2 Discovery-big](image/search-page.png)

### Scan the code connection

Entering the connection phone page, you can use the kylin-connectivity app to scan the QR code of the kylin-connectivity display to connect.

![Figure 3 Scan Code-big](image/mobile-scan-code.png)

### USB linkage

Enter the USB connection, select the corresponding equipment model configuration tutorial to set up the USB of the equipment, set up the connection to the USB and identify the kylinos, and click the immediate connection button to connect.

![Figure 4, USB-big](image/usb-connect.png)

### Device code is connected

Enter the device code connection interface, enter the device code displayed on the kylin-connectivity home page or app home page, and then click to connect immediately.

![Figure 5 Equipment Code-big](image/code-page.png)

## File sharing

After the successful device connection, kylin-connectivity can support file search, open file viewing, and file download and upload.

### File search

Click the search navigation bar search button to pop up the search box to search for all accessible files of the connected device.

![Figure 6 File Search-big](image/file-search.png)

### File View

kylinos display multimedia files on Android devices and files stored inside the phone for quick browsing. The kylinos and the kylinos will only display the user directory of files for sharing, double-click the file or right-click menu to open the file for view.

![Figure 7 File List-big](image/mobile-file-page.png)

### Files for upload and download

After the kylin-connectivity connection device, you can select the multi-end paired-end device file, click the download to the local button to download the file to the local device, or drag the local file into the kylin-connectivity storage directory and upload the file to the paired-end device.

![Figure 8 File Download-big](image/download-file.png)

### File Manager plugin

There is a second entrance of kylin-connectivity in the file manager. After a successful connection, you can enter in the kylin-connectivity option in the left navigation bar of the file manager to quickly view the file information of the paired-end device.

![Figure 9 File Manager-big](image/peony-file.png)

## Screen sharing & reverse control

For screen sharing, you need to distinguish between Android devices and kylinos.

### Android Device & Kylinos

After the Android device and kylinos are successfully connected, click the phone to cast the screen to synchronize the phone screen and sound (above Android Q synchronous system sound, below Android Q synchronous mic sound) to the computer. The screen projection window shows the bottom three keys navigation bar to support the reverse control operation (ADB connection), click X to close the screen projection and sound synchronization, support full screen and top set operations, the screen can rotate with the mobile phone screen rotation.

![Figure 10 Android screen casting-big](image/mobile-screen.png)

### Kylinos & Kylinos

The kylinos and the kylinos connection is different from the Android device connection, the connection initiator will click on the computer to cast the screen, and will share their own screen to the opposite end device. The recipient can use the suspended tab above to reverse the screen, return to the desktop, and end the screen sharing.

![Figure 11 Kirin screen-big](image/pc-screen.png)

## Disconnect

Click the Disconnect button to disconnect