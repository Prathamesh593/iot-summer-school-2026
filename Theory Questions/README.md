
# Theory: Difference Between Git Clone, Git Fetch, and Git Pull

# Git Clone
What it does: This command creates an exact, full copy of a remote GitHub repository on your local computer. It downloads all the project files, the entire commit history, and all existing branches. It also automatically sets up a tracking link called origin that points back to the source repository.

When to use it: You use git clone as your very first step when starting work on an existing project. For example, when you join a new project or want to work on a partner's repository, you clone it once to get a local working environment set up on your machine.

# Git Fetch
What it does: This command acts as a safe "check-in" with the remote repository. It reaches out to GitHub and downloads any new history, commits, or branches that your teammates have pushed, but it does not alter your current working files or merge anything into your active branch. It simply updates your local Git database tracking files.

When to use it: You use git fetch when you want to see what changes have been made on the remote repository without risking breaking your current local code. It allows you to safely inspect the remote status and compare histories before deciding to integrate the new updates.

# Git Pull
What it does: This command is an automated combination of two steps: it first runs git fetch to download the latest updates from the remote repository, and then it immediately runs git merge to fuse those remote changes directly into your current local active branch.

When to use it: You use git pull frequently throughout the day when you are ready to update your local files with the latest progress from GitHub. It keeps your local repository synchronized with your team, though you should ensure your local changes are committed first to avoid sudden merge conflicts.


# Theory: The .gitignore File
A .gitignore file is a text file placed in the root directory of a Git repository. It tells Git explicitly which files or folders to ignore, meaning Git will deliberately skip tracking changes to them.

This prevents large, temporary, or machine-specific compiled build files from cluttering your commit history and bloating your remote GitHub storage.


# Q13. Arduino Pin Types

## 1. Digital Input
**Explanation:**  
Digital input pins are used to read signals that are either **HIGH (5V)** or **LOW (0V)** from external devices.

**Example:**  
- Push button connected to **Pin 2**

**Real IoT Use Case:**  
A smart doorbell uses a digital input pin to detect when the doorbell button is pressed and sends a notification to the homeowner.

---

## 2. Digital Output
**Explanation:**  
Digital output pins are used to send **HIGH** or **LOW** signals to control electronic components.

**Example:**  
- LED connected to **Pin 13**

**Real IoT Use Case:**  
A smart home lighting system uses digital output pins to switch LEDs or relays ON and OFF remotely.

---

## 3. Analog Input
**Explanation:**  
Analog input pins (**A0–A5**) read varying voltage levels between **0V and 5V** and convert them into digital values ranging from **0 to 1023**.

**Example:**  
- Potentiometer connected to **A0**

**Real IoT Use Case:**  
A smart irrigation system uses a soil moisture sensor connected to an analog input pin to monitor soil conditions and automatically control watering.

---

## 4. PWM (Pulse Width Modulation) Output
**Explanation:**  
PWM pins (**3, 5, 6, 9, 10, and 11**) generate pulse signals that simulate analog output, allowing control of LED brightness or motor speed.

**Example:**  
- LED connected to **Pin 9** with adjustable brightness

**Real IoT Use Case:**  
A smart street lighting system adjusts LED brightness automatically based on ambient light conditions to save energy.

---

## 5. I2C/SPI Communication Pins
**Explanation:**  
These communication pins allow the Arduino UNO to exchange data with sensors, displays, and other peripherals.

### I2C Pins
- **SDA:** A4
- **SCL:** A5

### SPI Pins
- **SS:** Pin 10
- **MOSI:** Pin 11
- **MISO:** Pin 12
- **SCK:** Pin 13

**Example:**  
- OLED Display
- MPU6050 Accelerometer Sensor

**Real IoT Use Case:**  
A smart weather station uses I2C or SPI communication to collect data from multiple sensors and display the information or send it to the cloud.




