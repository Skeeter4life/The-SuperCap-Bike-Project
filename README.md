# SuperCAP V1 is Complete, and Deprecated in light of V2.

## The Idea

I wanted an electric bike with a more dynamic feel than the typical e-bike; one in which you wouldn’t be able to get away with constantly cruising at 30km/h with minimal effort. I wanted an e-bike that had a sense of “sportyness”, quick, instantaneous acceleration. I also wanted the sence of scarcity. The goal was to have an e-bike that could get me up a hill, yet had limited capacity, such that it still enticed me to get some proper exercise. A super-capacitor based accumulator is perfect for this, as they have reduced charge density, and generally are able to source and sink signifigantly more current with less problems then for instance, a Lithium Ion battery. The accumulator would consist of two 12V 500F supercapacitor banks, which could be connected in parallel or in series. The electronics will be powered independently with a 12V battery.

> [!Note]
> These were my honest initial thoughts going into the project. The practical implications of this will be described in [Things That Went Wrong](#things-that-went-wrong)
>
> Inspiration  from [Tom Stanton's "Super Capacitor Bike"](https://www.youtube.com/watch?v=V_f8Q2_Q_J0) video on YouTube.

> [!Note]
> I am going to focus my discussion here on the hardware. There is so much to talk about regarding the firmware, but to keep this documentation of my learning as condensed as possible, I will rely on my comments I made in my code for you to interpret. 


> [!IMPORTANT]
> The uploaded schematic is original, and has NOT been updated to reflect any of the changes mentioned below. I have decided to use that time towards V2.

## Things That Went Right

### 1. Dedication and Perseverance
It is very important to know that I have had no formal education on any of this (as of 2025/11/08). I have not yet taken any course that explains the practical applications and operation of a diode, MOSFET, or any nonlinear device. I spent my evenings teaching myself as much as I could before falling asleep. I spent as much energy and time as I could during my free time (sometimes putting off homework…) to work on my project. I need to emphasize that I was extremely dedicated and determined to get this project in a usable state. I am only now taking ECE 370 this term, which sheds light on the operation of electromechanical devices.

I also did not have any adequate equipment when going into this project. I did not let that stop me, but reflecting back on it, it would have made a world of difference. Please, behold, the setup I had in my landlord’s garage while I was on co-op:

![Setup-landlord](https://github.com/user-attachments/assets/584c68bb-495f-48c9-82ee-e2c88a23779d)

### 2. Decision to use a 2 Layer PCB for Prototyping
If you looked at this project in the early stages of development, you may remember that I chose the 2-layer PCB primarily due to cost and “EMI was not my primary concern” (Okay, that was really silly to say. Hindsight is 20/20). This will be briefly addressed in [Things That Went Wrong](#things-that-went-wrong), to an extent, as far as I can explain; I am not even close to being able to design for EMC. This choice ended up being quite beneficial as I needed to cut and re-route many, many traces on my PCB.

### 3. I had the Right Idea
I understood the principles of what needed to happen, and how it could be achieved. While some components could definitely have been chosen better ([Things That Went Wrong](#things-that-went-wrong)), atleast they were the correct type of components. I understood that I needed 3 half bridge circuits, one per phase. I understood that a P-type MOSFET typically has a higher $$R_{ds ON}$$ than an N-type MOSFET. I understood that I needed some sort of circuit to keep $$V_{gs}$$ in the saturation region as the high-side N-type MOSFET began to conduct. I understood the principles of decoupling capacitors, I choose appropriate values and kept them as close as possible to the IC. I even put bulk capacitance throughout my circuit (could have done a better job, but I will still give myself props). Furthermore, I even tried to put some (questionable at best) circuit protection features. This prototype defiently would not even be workable if I did not execute those parts properly.

## Things That Went Wrong 

As mentioned in [Things That Went Right](#things-that-went-right), I was completely self taught. I wish I had the time to learn absolutely  everything, tinker with each component for way longer, but things have been way too busy. Part of me just wanted this project done for the next co-op search and to finish my schoolwork, so, I rushed. There are many design flaws, logic errors, and inconsistencies throughout the project; but to keep this section concise and readable, I am going to limit myself and only discuss in detail the three most signifigant design faults. 

### 1. No ISP

This ultimately came down to an extremely silly error: I used 1.0mm header footprints, not 2.54mm. I had the pins to program the ATMEGA328P over UART, but due to their size, spacing, and my available tools, it made it way too finicky to solder on a bodge fix. As such, I had to remove the ATMEGA328P and place it onto a breadboard each time to reprogram, which is a mistake I do not want to repeat (The 28-PDIP IC defiently was not designed for constant insertion and removal). Additionally, my screw terminals were missing the SPI line MISO and a connection to the ATMEGA328p's RESET pin, so I had to repeat the process of removing the microcontroller each time to use DebugWIRE or change a fuse bit.

### 2. Chosen Components and Thermal Performance

I mentioned this before in [Things That Went Right](#things-that-went-right), that I did my best to figure out what all of the specifications meant on the datasheet, but it ultimetly meant that some paramaters got neglected. It is crazy to say, but sometimes I got so hooked after learning something nitty-gritty about a component, that I got excited and put it on my BOM, and completely neglected some other paramaters. For instance, let's take a look at my choice for using the onsemi 1N4148 signal diode [^5] used in the bootstrap circuit for the IR2104 [^30] (mistakenly, it has a schottky diode symbol on my schematic). 

<img width="1097" height="394" alt="image" src="https://github.com/user-attachments/assets/dab5a638-39e2-4f7a-a242-9907bd363223" />

The first thing that pops out to me is the absolute maximum recurrent peak forward current rating of 400mA. I likely exceed this during the initial C<sub>boot</sub> charging transient, as I do not have a current-limiting resistor in series with the diode. Below entails an analysis to see the current drawn at t = 0:

<p align="center">
  
$$ I(t) = I_oe^{\frac{-t}{\tau}} \text{ (A)}$$

$$ I(0) = I_o = V/R_{eq} \text{ (A)}$$

$$R_{eq} = R_{diode-eff} + Capacitor_{ESR} + Sourse_{ESR} + Parasitic_{ESR}\ (Ω)$$

Assumption: $$R_{diode-eff} = 0$$
I am assuming that the diode is an ideal diode to get an upper-bound for the peak surge current. There would be a forward voltage drop and some equivalent effective resistance at that current, but the datasheet does not provide any information as soon as the current $$> 800mA$$.

$$Capacitor_{ESR}: \ \approx \text{15 (mΩ)}$$

$$Sourse_{ESR}: \ \approx 8 * 0.2 = 1.6 \text{ (Ω) (8 1.5V AA alkaline batteries in series)} $$

$$Parasitic_{ESR}: \text{ The resistance of a trace with 1.5mm width, 0.6mm length, } 1 \frac{oz}{ft^2}, \text{ at } 25^{\circ}C \approx 0.194 \text{ (mΩ).}$$

IR2104 I/O impedence assumed neglegable.

$$R_{eq} \approx 1.62 \text{ (Ω)}$$

$$= 12/1.62 = 7.4 \text{ (A)}$$

</p>

However, the surge current decays quickly as seen at three time constants:

$$ I(t) = I_oe^{\frac{-t}{\tau}} \text{ (A)}$$

$$ I(3\tau) = \frac{I_o}{e^3} = V/(R * e^3) \text{ (A)}$$

$$= 12/32.54 = 369 \text{ (mA)}$$

Now let's take a quick look at its thermal characteristics:

<img width="1130" height="167" alt="image" src="https://github.com/user-attachments/assets/566f0709-05cb-4a90-8187-37ec026ba1af" />

It is quite likely I exceeded the power dissipation rating aswell. Now it is true that this tiny diode was dissipating power for a tiny amount of time: 

$$5\tau \text{ (s)} $$
$$\text{Voltage } C_{boot} = 0.993(12) \text{ (V)}$$
$$ = 5*RC_{boot} $$
$$ \text{= 5 * 1.62 * 0.000001 = 8.1 (us)} $$

So, it almost makes some sense that it did not explode as soon as I tried using my bootstrap circuit. The instantaneous power is quite high, but the average over time with the frequency I was switching the MOSFETs (~10kHz: a limitation of the timers in the ATMGEA328P) is acceptable. Nevertheless, its vital to design a system that meets all absoloute maximum ratings, with a buffer. 

Furthermore, The 1N4148 diode has a non-negligable forward drop when sinking even uA of current. The highside is still being driven at an appropriate $$V_{gs} \approx 11V$$. However, this is still worth additional consideration moving forward. 

<p align="center">
<img width="469" height="404" alt="image" src="https://github.com/user-attachments/assets/314d42fe-a0ac-4809-ba37-bde894e269af" />
</p>

On the other hand, I do want to briefly go over the fact that the 1N4148 diode's maximum repetitive reverse voltage, reverse leakage current, reverse recovery time and junction capacitance specifications are appropriate for this use case. When the IR2104's high side gets switched on, the 1N4148 is going to need to handle a reverse voltage of 24V. It is rated for 100V, so that is a win. The only inductance in the loop is from the parasitics from the traces, so $$L \frac{di}{dt}$$ is small, and thus, the switching transient will remain well below 100V. The 1N914 has a reverse leakage of 0.025uA when $$V_R = 20V$$, which means I am not going to get too heavy in the math, but to my understanding, low leakage current means the bootstrap capacitor will stayed charged for longer, and the low junction capacitance/fast recovery time minimizes the ringing/switching loss, which is good.

I think that covers everything for the bootstrap diode. My math definitely overestimates the stress on the diode through my ideal diode simplification, but all in all, this is a better analysis than what I had before. If I were to choose any other component to replace it, under the following assumptions:

- Able to handle a repetitive surge forward current of a of couple amps, allowing for some $$C_{bs}$$ to be charged in the order of a few nanoseconds,
- Required reverse votlage of 24V * 2 (accumulator in series, 2* to give room for transients),
- Fast recovery time, minimizing losses
- Low leakage current
- Low forward voltage drop

I would choose [this Schottky diode](https://www.digikey.ca/en/products/detail/taiwan-semiconductor-corporation/SSH210H/18718578).

### 3. The MCP23017

To begin, a better choice would have been an I/O expander with a protocol capable of running at a higher baud rate, like SPI. One of my greatest mistakes in this project was not considering the timing implications of using the I/O expander, the MCP23017. I wired the digital output of hall effect sensors, break switches, and even the fault LED to the I/O expander. These are all things that I would want directly connected to the MCU. The hall and break switch interrupts are extremely high priority. The MCP23017 has two interrupt lines, multiplexed across the two ports, dedicating 8 independent pins per port. The designers of the MCP23017 added a register that holds the flags for the pins that raised an interrupt; however, this creates a major performance issue in my case. It is required to read the “interrupt captured register” before the interrupt line state resets. Every interrupt required a full read cycle, which is a major performance issue: the I2C bus for this application is unsafe, unreliable, and causes atleast a 10 times delay of the MCU’s ability to process subsequent interrupts. I originally did not consider this delay. As a result, the motor would stall at speed; the hall state would be read too late, and a phase with a lower index would be applied.

I did a brief analysis of how much time I had between subsequent hall states. Below is an image of the motor rotated to one of its commutation states:

<p align="center">
<img width="563" height="677" alt="image" src="https://github.com/user-attachments/assets/41260995-e4db-4e6a-ad3f-9ebcdb4b13fc" />
</p>

The radius I measured to be about $$50mm$$. Using a protractor, the rotor only needs to turn $$\approx 1^\circ$$ before the hall effect sensor changes state. The calculus for measuring the arc length of a circle has been done and is well known: 

$$L = 2\pi r \frac{\Theta}{360}$$

$$L \approx 0.87 \text{ mm}$$

Below is an image of the fastest possible I2C read transaction with the ATMEGA328P, 

(I consider this a feat, I am quite proud of this. Bit rate of ~1MHz (2f).

<p align="center">
<img width="1024" height="600" alt="SDS804X_HD_PNG_9" src="https://github.com/user-attachments/assets/9e760ce3-5df8-42db-a15a-d434d97d4c3e" />
</p>

A bit of algebra yields the absolute maximum I2C interfaced motor speed:

$$\frac{0.87*10^{-3}}{v} \le (67.8 * 10^{-6}) + ISR_{execution} $$

$$\text{For simplicity (was not measured) } ISR_{execution} = 0$$

$$ v \le 12.8 \frac{m}{s} \approx 45 \frac{km}{h}$$

If you are familiar with I2C, you would have noticed that the ACK bit had a suspiciously high voltage. At the speeds I was running it, it would occasionally glitch and the MCP23017 would not be able to pull the ACK bit low, resulting in a NACK bit. Using stronger pull-up resistors did not make a noticeable difference. I was pushing the limits of my I2C bus, and the AMTEGA328p’s TWI timing capabilities; it is officially only rated for a 400kHz transfer speed.

So, when doing my tests, I reduced my I2C baud rate from 1MHz to 400kHz. Here is a video demonstrating the motor stalling (video is heavily compressed to meet 10MB upload limit, apologies):

https://github.com/user-attachments/assets/7227a615-8b7e-4955-889b-a2fbc8cb35df

> [!Note]
> I bought the oscilloscope and tools after my co-op. The PCB was designed and built at this time.

Revisiting the first note, the main flaw with this supercapacitor bike idea is that none of my electronics or even my motor is rated for large currents. I don’t have the datasheet for my motor (I asked, but, the seller wouldn’t give it to me), but the use of 16-gauge stranded aluminum phase wires was enough for me to realize that sinking anything around 10A was a bad idea. The other key flaw with my original intuition was I could simply just decided not use the e-bike’s power assist by turning it off, nullifying the entire supposed benefit of the limited capacity. Some sort of hybrid capacitor-battery approach would be better suited (Lithium-ion Capacitors), whilst keeping the spirit of the project.

### Honerable mentions:

**1. Incorrect Wiring of the IR2104's**
  - All shutdown pins were tied together.
    
**2. Completely Inconsistent Labels, Silkscreen, and Ordering**
  - MCP23017's INTA is labeled as INTB, and vice versa.
  - U2 has silkscreen "U1", U1 has silkscreen "U2".
  - U1, U2, and U3 are not organized in a sequential order.
  - Phase1, Phase2, and Phase3 connections are not listed sequentially.
    
**3. Wrong Footprints**
  - Barrel Jack (PJ-002AH) FootPrint: A lesson to not put complete trust into the EDA models found on DigiKey. I had ignored KiCAD's DRC violation that the hole clearance was 0mm, shorting supply and ground.
  - Relay (AWHSH105D00G) Footprints: Wrong footprint, one through-hole pad is entirely missing.
    
**4. Circuit Protection Elements Doing More Harm Than Good**
  - I have diodes on my 5V, 3.3V lines; this caused the voltage to be too close to $$V_{min}$$ of the MCP23017's defined logic levels.
    
**5. Schematic is Unorganized and Difficult to Read**
  - Organizing a schematic takes expereince.
    
**6. No (initial) Consideration of the I2C Bus Pull Up Resistors**
  - I arbitrarily chose 10kΩ, which I later had to add 2.2kΩ resistors in parallel.
    
**7. Series/Parallel Switching**
  - To keep this brief, I feel that this is still a good idea (assuming software and hardware has been implemented to ensure it is safe to do so), but there needs to be a major revision to what I have on my schematic. The most prominent issue is the relays. EMI was such an issue on my PCB that under high load, or some sort of grounding issue, the relay(s) would toggle. If I were to ever choose to use relays, I would need to do proper analysis to ensure that under no circumstances can they spuriously toggle. Also, I would need to add some sort of current sensing to ensure that I am still operating within the contact ratings. Another major concern is the potential for inrush currents. It is quite plausible to have one capacitor at a slightly different potential than the other. In addition to the current sensing, there would be a need for some sort of regulator to handle transients.
    
**8. I Did Not Add Any Real Way to Charge the Accumulator**
  - I had the original idea of just taking the accumulator off, hooking it up to a DC power supply etc, but a nice luxury would be to be able to plug it in directly to a wall outlet.

**9. The PCB Could Have Been Designed Better**
- I rushed the production of the PCB, and I believe I could have done a better job routing tracks and organizing the components. The ground and power planes are excessively fragmented, resulting in poor current return paths and increased noise.
- The board already exceeds the 100x100mm pricing tier. Surface mount components would have reduced cost and complexity. There is an excessive amount of signal via's, traces bending around IC's with sharp corners, etc., that could have been avoided with better component placement, organization and additional board space. Additionally, I would have been able to better separate the sensitive digital electronics from any high power tracks and the power plane. 
-  I could have improved thermals and reduced capacitive coupling with the use of thermal and stitching vias. Furthermore, I should have kept the high-frequency SPI, I2C, and the crystal oscillator tracks as isolated, short, and straight as possible.
    
**10. No Pull-Down Resistors on Some MOSFETS**
  - I did not add any pulldown resistors on the half-bridge FET's. When the IR2104 driver was in a HIGH-Z state, capacitive coupling caused $$V_{gs}$$ to increase, just enough to start to put the FET into conduction. The only reason I noticed this is because some FET's got mysteriously hot.
    
**11. Bootstrap resistors, Gate Resistors Chosen Arbitrarily**
  - To be fair to myself, I had no equipment at the time to be able to emperically measure the effects of one gate resistor vs. another. However, these are the sorts of things you can get an upper and lower bound for through calculations, which I did not do.

There are definitely some more things I did not write, might have missed/don't fully understand yet. All in all, I will conclude this section here, as I believe it highlights the core of my learning. If there is something here you would like to talk about, feel free to in an interview or in the issues section.

## The Outcome

I consider this project to be a success. I had a lot of fun releasing some magic smoke. In all seriousness, I got some incredibly valuable hands on experience designing my first ever prototype for quite an ambitious project. I essentially have a fully functional BLDC motor controller. Below, I posted an uncompressed video of one of the "top speed" runs of my motor (or at least, as fast as I can go without feeling worried about tripping my room's breaker or melting those aluminum wires).

<div align="center">
  <video src="https://github.com/user-attachments/assets/f553f186-a5bf-4020-b508-6ff7d6a13e7c">
</video>
</div>

(Note: I was using a higher duty cycle than the I2C test) Below is an image of the bottom my PCB with the "bodge" fixes:

<p align="center">
<img width="509" height="448" alt="PCB_Bodge" src="https://github.com/user-attachments/assets/f8798f4a-d251-4ce3-af1b-76125cb8b6df" />
</p>

Below is a top down view of the PCB:

<p align="center">
<img width="509" height="448" alt="PCB_Bodge" src="https://github.com/user-attachments/assets/8e145258-7763-4d6a-8033-fa55f43c99e4" />
</p>

## What I Will do in Version 2

Version 2 will address all of the aforementioned issues. Here are my initial thoughts and goals:

- I will switch to a more capable microcontroller. I am definitely looking into STM32, I need something with a higher instruction throughput, greater word size, ideally has an FPU, variable frequency timers (capable of field-oriented control), and more I/O. Everything should be able to interface directly to the MCU. I need to include relevant ISP modules.
- I need to switch to a larger 4-layer or 6-layer PCB, and improve my designs layout and organization. I will attempt to reduce EMI: I should have dedicated uninterrupted ground planes, stitching vias, and use best practices to keep high-frequency traces short and straight. I also need to keep thermals in mind.
- I will use heat sinks where needed and thermal vias; stitching the top and bottom copper planes.
- I will perform power and EMI analysis (I will be switching to Altium).
- I will design a proper back-EMF circuit for regenerative braking; power dissipation in body diodes will be high and inefficient.
- I will switch to a Li-ion capacitor-based accumulator. I will design a circuit to allow for charging directly from a standard wall outlet. This of course requires safe charging/protection features.
- I will design a current and voltage monitoring system to ensure the motor is operating safely, and to take action when it is not.
- I will aim to drive a 500W BLDC motor (a motor *with* a datasheet). I will consider the motor's characteristic curves to maximize power output and efficiency.

## Bill of Materials

Ceramic Capacitors: 
- 0.1uF x 7 [^1], 
- 0.33uF x 2 [^2],
- 1uF x 3 [^3]

Electrolytic capacitors: 
- 10uF x 4 [^4]
  
Diodes:
- 1N4148 x 3 [^5], 
- 1N5819 x 5 [^6]

LEDs: 
- 5.0mm THT x2 [^7]

Fuse: 
- 0225002.MXP x1 [^8], *Note:* Never implemented or tested.
- F4658-ND x 1 [^9] *Note:* Never implemented or tested.

Sockets & Pins: 
- 1x01 1.00mm Vertical Male Pin x 5 [^10],  *Note:* Never purchased, implemented or tested.
- 8 Dip 7.62mm Socket x 5 [^11],
- 16 Dip 7.62mm Socket x 1 [^12], 
- 28 Dip 7.62mm Socket x2 [^13] 

Barrel Jack: 
- PJ_002AH x 1 [^14]

Relays: 
- AWHSH105D00G x 2 [^15]

MOSFETS:
- IRFZ44N x 7 [^16], 
- GT065P06T x 1 [^17]

Resistors: 
- 0Ω, 10Ω, 220Ω, 10kΩ, 22kΩ Resistor Kit: [^18]

Terminals: 
- 1988804 x 1 [^19], 
- 1988817 x 2 [^20], 
- 1190363 x2  [^21], 
- 1988862 x 3 [^22]

Microprocessors: 
- ATMEGA328P x 1 [^23]

Crystal Oscillator: 
- MXO45HS-3C-20M000000 x 1 [^24]

GPIO Expander: 
- MCP23017 x 1 [^25]

Button: 
- TS02-66-50-BK-160-LCR-D x 1 [^26]

MOSFET Gate Driver: 
- UCC37324P x 1 [^27] *Note:* Never implemented.

Darlington Transistors Array: 
- ULN2003 x 1 [^28]

Digital Potentiometers: 
- MCP4151 x 1 [^29]

Half Bridge Drivers: 
- IR2104 x 3 [^30]

Linear Regulators: 
- L7805 x1 [^31], 
- L78L33ACZ x1 [^32] *Note:* Never purchased, implemented or tested.

Hall Effect Sensors: 
- TMAG5213AGQLPG x 3 [^33]

Switches: 
- 2057-SW-R-K1-A-ND x 1 [^34] 

Subtotal: **$ 146.17 CAD** (as of 2025/11/08)

## Schematic and PCB

### Schematic:  

  To reduce image congestion, please clone the repository and access the schematic on KiCAD version 8+.

### PCB:

<div align="center">

<img width="1007" height="1138" alt="PCB_Components" src="https://github.com/user-attachments/assets/e85fb27b-7a71-4477-bf4e-62457685efae" />
<img width="1099" height="1190" alt="PCB_Traces" src="https://github.com/user-attachments/assets/334e2562-5c2c-4941-808b-48a1f9e39c9b" />

</div>


## My Policy on the Use of AI

I fully acknowledge the emergence of AI and how useful it can be as a tool. However, I am also fully aware that there are negative consequences for the overreliance on AI. That being said, I take pride in the fact that I fully wrote all core components of my code; the same is said with the hardware portion. If you look carefully through my program, you will be able to pick up my quirks and coding style. However, I did not completely abstain from using AI. I would ask ChatGPT to help me with a concept, look up what register to use, or how to perform some sort of function. I used ChatGPT as a tutor, not as a tool to do everything for me. It is worth mentioning that ChatGPT is not perfect. For instance, I pasted some code into ChatGPT and asked “what interrupt vector do I use here?”, to which ChatGPT said “INT_vect” instead of “INT0_vect”. A very minor mistake, but I didn’t understand what was wrong, and I was using no other resources. I was then confused for an hour about why my ISR never executed. It was confidently incorrect the entire time we went through the debugging process; I was overreliant, and ChatGPT was more concerned with my wiring setup than realizing the interrupt vector was wrong. So, to summarize, AI is a great tool, and there are parts of my code where AI helped me out, but I never just blindly copied and pasted big chunks of its work for a full feature. I spent countless hours going through datasheets (I feel like I am one of the very few people who have read nearly the entirety of the ATMEGA328P datasheet), did my best to find information myself, and then, if all else failed, I would have asked ChatGPT for some advice.

<div align="center">

  ***Thank you!***

</div>

![Hall-Effect-Sensor-Replacement](https://github.com/user-attachments/assets/c2a49036-e808-4c4e-8cd6-5d72a864b381)

## Links

[^1]: https://www.digikey.ca/en/products/detail/kemet/C322C104M5U5TA/818107
[^2]: https://www.digikey.ca/en/products/detail/kemet/C330C334M5R5TA/6562403
[^3]: https://www.digikey.ca/en/products/detail/kemet/C330C105K5R5TA7301/3726162
[^4]: https://www.digikey.ca/en/products/detail/cornell-dubilier-knowles/106CKE400M/5410625
[^5]: https://www.digikey.ca/en/products/detail/onsemi/1N4148/458603
[^6]: https://www.digikey.ca/en/products/detail/stmicroelectronics/1N5819/1037326
[^7]: https://www.digikey.ca/en/products/detail/marktech-optoelectronics/MT4118-HR-A/4214622
[^8]: https://www.digikey.ca/en/products/detail/littelfuse-inc/02540101Z/553102
[^9]: https://www.digikey.ca/en/products/detail/littelfuse-inc/0225002-MXP/777781
[^10]: https://www.digikey.ca/en/products/detail/samtec-inc/TS-101-T-A/1105493
[^11]: https://www.digikey.ca/en/products/detail/te-connectivity-amp-connectors/1-2199298-2/5022039
[^12]: https://www.digikey.ca/en/products/detail/on-shore-technology-inc/ED16DT/4147596
[^13]: https://www.digikey.ca/en/products/detail/adam-tech/ICS-328-T/9832859
[^14]: https://www.digikey.ca/en/products/detail/same-sky/PJ-002AH/408446
[^15]: https://www.digikey.ca/en/products/detail/amphenol-anytek/AWHSH105D00G/16721986
[^16]: https://www.digikey.ca/en/products/detail/umw/IRFZ44N/24889964
[^17]: https://www.digikey.ca/en/products/detail/goford-semiconductor/GT065P06T/18088004
[^18]: https://www.digikey.ca/en/products/detail/sparkfun-electronics/COM-10969/14671649
[^19]: https://www.digikey.ca/en/products/detail/phoenix-contact/1988804/950892
[^20]: https://www.digikey.ca/en/products/detail/phoenix-contact/1988817/950893
[^21]: https://www.digikey.ca/en/products/detail/phoenix-contact/1190363/14680902
[^22]: https://www.digikey.ca/en/products/detail/phoenix-contact/1988862/950898
[^23]: https://www.digikey.ca/en/products/detail/microchip-technology/ATMEGA328P-PN/2357094
[^24]: https://www.digikey.ca/en/products/detail/cts-frequency-controls/MXO45HS-3C-20M000000/1801871
[^25]: https://www.digikey.ca/en/products/detail/microchip-technology/MCP23017-E-SP/894272
[^26]: https://www.digikey.ca/en/products/detail/same-sky-formerly-cui-devices/TS02-66-50-BK-160-LCR-D/15634352?s=N4IgTCBcDaICoGUAMYC0A2dqCsTUCEBpVARnTwBkBhAJVQBEACEAXQF8g
[^27]: https://www.digikey.ca/en/products/detail/texas-instruments/UCC37324P/571464
[^28]: https://www.digikey.ca/en/products/detail/stmicroelectronics/ULN2003A/599603
[^29]: https://www.digikey.ca/en/products/detail/microchip-technology/MCP4151-103E-P/1874217
[^30]: https://www.digikey.ca/en/products/detail/infineon-technologies/IR2104PBF/812198
[^31]: https://www.digikey.ca/en/products/detail/stmicroelectronics/L7805CV/585964
[^32]: https://www.digikey.ca/en/products/detail/stmicroelectronics/L78L33ACZ/1038304
[^33]: https://www.digikey.ca/en/products/detail/texas-instruments/TMAG5213AGQLPG/22531584
[^34]: https://www.digikey.ca/en/products/detail/adam-tech/SW-R-K1-A/15284469
