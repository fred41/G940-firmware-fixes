# G940-firmware-fixes    
     
G940 flight system, firmware related improvements.    

The G940 Flight system, was released around 10 years ago, but is still a very interesting and unique device.    
      
There are some mechanical and some firmware related flaws though. Some users developed and published      
simple fixes and workarounds, for most of the mechanical issues.    
      
The firmware related problems are partly fixed in the latest (unofficial) firmware update 1.42.    
Nevertheless all analog axes, exept the two main stick axes, are still showing a strange 'hysteresis' behavior.    
     
I debugged the firmware (in system, gdb, st-link) and finally improved the following properties of the input part:    
- precision of all 8-bit axes significantly improved (reversal bug fixed)        
- removed the firmware 'deadzone' from the rudder axis         
- increasing the active stick zones (more movement in x and y directions)      
- main axis precision improved by optimizing the noice filtering function (true/minimal hyteresis now)    
- some minor optimizations       
       
Force feedback related improvements are:     
- adapted the minimal pwm duty cycle to minimize the unwanted deadzone (center play)     
- adapted spring coefficient to saturate simultanous with min/max control positions at 100% setting        
- adapted the default condition effects params to be less "aggressive"     
     
To use this patch you need the following:    
     
- a copy of the firmware updater 'G940_Update_FW0142.exe' (don't forget to make a backup)       
- 'g940_patch.exe'    
     
To update your G940, you can do the following:     
1. your device should be already be updated with firmware 1.42    
2. put both executables in the same directory and launch 'g940_patch.exe'    
3. after that, start the patched 'G940_Update_FW0142.exe' to update your G940    
     
How does it work?     
The firmware updater 'G940_Update_FW0142.exe' contains the firmware as a binary blob.    
'g940_patch.exe', if applied, directly changes some instructions and data in this blob.    
By using this patched 'G940_Update_FW0142.exe', you write a modified firmware to your device.    

This patch is open source, so if you are familiar with arm MCU's (cortex M3) and able to use a dissassembler,    
you will be probably able to understand my modifications.     
But better don't change anything, if you don't know what you are doing!!!    
      
DISCLAIMER:
By using this patch you agree to the following terms and conditions:      
I (the author of this patch) am in no way responsible for anything that is caused by this patch.      
    
