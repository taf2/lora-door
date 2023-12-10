# Garage Open/Closed

This project attempts to provide a sensor system to indicate from some distance that our garage door is either open or closed.


# How it works

We'll use two lora boards feather m0 from adafruit should do the trick.

We'll have a positional sensor that let's us know if the door is open or closed

When open is detected the attached feather will broadcast a signal to any one listening via lora radio "O"

When the door is detected closed we'll broadcast "C"


# Setting up

we need platformio
```
python3 -m venv env/
./env/bin/pip install platformio
```

```
run --target-upload
```


## for linux:

Create a rule in /etc/udev/rules.d that will set the permissions of the device (a restart will be required):

```
  # navigate to rules.d directory
  cd /etc/udev/rules.d
  #create a new rule file
  sudo touch my-newrule.rules
  # open the file
  sudo vim my-newrule.rules
  # add the following
  KERNEL=="ttyACM0", MODE="0666"
```
