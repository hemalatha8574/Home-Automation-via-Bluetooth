
---

# Home Automation using Bluetooth

## 🎯 Objective  
To control home appliances via smartphone Bluetooth commands.

---

## 🛠 Components Required  
- Arduino UNO  
- HC-05 Bluetooth Module  
- Relay Module  
- Bulb/LED (Appliance)  
- Smartphone with Bluetooth app  

---

## 🔌 Circuit Diagram  

HC-05 TX ---> Arduino RX (D0)
HC-05 RX ---> Arduino TX (D1)
Relay IN ---> Arduino D8
Relay NO ---> Appliance



---

## ⚙️ Working Principle  
- Mobile app sends data ('1' = ON, '0' = OFF).  
- Arduino receives via Bluetooth → controls relay → appliance ON/OFF.  

---

## 🌍 Applications  
- Smart homes.  
- Wireless control for elderly/disabled.  
- Office automation.  

---

