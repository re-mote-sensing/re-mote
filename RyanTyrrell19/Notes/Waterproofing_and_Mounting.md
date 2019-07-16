# Outline
* Waterproofing Methods
* Mounting Methods


# 3D Printing Notes

## Printing Material

* Materials available - PLA (Makerspace & Mills)
	* tend to soften and eventually dissolve if exposed to water for any length of time. [[2]]

## Waterproofing

*  In these cases you should consider post processing your print to seal the surface. The **ABS smoothing technique with acetone** could do this, or you could add an **epoxy mix like XTC-3D to smooth and seal the surface**. Finally, there are a number of **conventional sealants one could use, including paint**. [1]

* Plastic Spray paint
* Conformal coating
* Painting the print with epoxy

# Enclosure Notes
## Most plausible setup

3 pieces total - a permanent 'base' that will always remain attached to the turtle, and a bottom section & a top section that form the enclosure

* CAD design made up of 4-pieces - 2 end pieces, bottom & top piece
	* Bottom  case has a slight curve on the bottom, curved ends, narrow. The curvature of the bottom can be altered based on the turtles shell. It's mostly solid except for a hollow rectangular shape on the inside where the battery will fit into. One end is slightly longer and flatter for the Boards antenna to lay.
	* End pieces are customized based on the turtles shell. 
	* Top piece is more of a shell then a case. It covers the board and GPS. The top is non-existent, allowing the GPS to be uncovered. It has a slight curved shape 
* 4 snap-fit joints will be included on the top case, facing inwards (so the outside of the case remains smooth).




## Ideas

* Alterations to case
	* If needed, the top case can be cut away slightly to expose the LoRa antenna. This can be done if the casing blocks the signal (although it'll make the casing less streamlined) 
	
* Top and bottom attachment ideas
	* Permanent solution; gluing the two together
	* Attach using joints; snap-fit joint
	* Sliding joint; top case into the bottom case
		* snap-joint to lock it in; 
			* on the back to lock it; accessable to unlock from the bottom. it clips in as the top case slide inwards
			* on the side; accessable to unlock from the sides. it clips in as the top case slide inwards
			* on the front; accessable to unlock from the front. it clips in as the top case slide inwards

* Permanent attachment idea
	* **NOTE**: 
		* will need some sort of buffer between the plastic and the turtles shell
		* 4 small pieces will protrude from these 'bases' that will act as mounts for the epoxy.
	* A thin rectangular outline of the Tracker with 4 small, thick snap-joints protruding from it. The bottom of the Tracker enclosure would snap into it. 
		* May be difficult getting the curvature right
	* A flat rectangular piece thick enough to match the curvature of the shell, and include a sliding-joint for the bottom case to slide into
		* A snap-joint must be included somewhere (front, side, back) to prevent the enclosure from sliding out
	* Bottom piece slides into centre piece on base, rotates and locks into place (perhaps using 2 snap-joints at the ends)

## To consider
* Cost
* Weight
* Is it printable?
* Ease of use
* Airtight?


* Tree diagram that will choose a design based on epcs (e.g. additional piece that is permanently on turtle that the enclosure attaches to, etc.)

# Mounting on the Turtle Notes
* Ideas
	* Entire enclosure is removable from turtle?
	* An attachment is permanently on the turtle, and the enclosure attaches to this attachment




# Waterproofing/sealing Methods

* Epoxy
	* Smear some epoxy over any cracks to quickly make it watertight. Easy to remove (just scrap away at it)
* petroleum jelly (Such as vasoline)
	* " I used it on my spruce moisture sensor in the grass. Working great so far. Does not dry and keep moisture out of the sensor" [1]
* Plastic wrap (Saran Wrap) / Food Vacuum Sealer / Shrink Wrap
* Rubberized Spray Paint
	* " I used the rubberized spray paint. I saw the commercial on tv with some watermelon drop or something but you can get it at Lowes and home depot. they have white so it looks normal. I also like that I can scratch it off with my nails when I need to change batteries. The tv brand is called flex seal, but just ask for the rubberized spray paint as they all make a version now" [1]
* Silicon Conformal Coating


# Additional Notes

* Thermal considerations
	* "A very common oversight is the lack of thermal analysis. In particular, for very low-power battery-operated equipment, it’s easy to think that the power levels are not high enough to worry about thermal management. However, remember that the power density of these handheld systems can actually be quite high" [1]
	* " There are boxes specifically made to “breathe” without letting water in."[1]
	* " Yes, heat affects electronics in many different ways, especially lifespan.   
But we are talking about a sensor that stays on for a couple of milliseconds at most, to read the temp and send a quick data pulse. How much heat is being generated? I’m going to guess, very very little.  
The part that really affects the electrons is simple, there are two issues…  
First - prolonged power consumption without proper cooling, think CPU.  
Second - the fluctuating temp of a device turning on and off. Changing from 75° to 120° does more damage than anything." [[1]]
	* Heat buildup from battery. Main concern is heat buildup if pinpointed to a specific area (rather then the entire unit heating up)
* Setting influence on the Tracker + Enclosure
	* Underwater; heat should dissipate quickly
	* Above water, direct sunlight; turtles sunbath. Tracker could become quite hot if in the sun for a few hours (plastic may melt, battery may overheat, etc.)
* ** Battery (or entire tracking unit) should have an additional waterproof layer to ensure it does not malfunction**

# 3D Printing Tips
## Snap-Joint

![alt-text][Cantilever Snap Joint]  
![alt-text][Snap Fit 1]
![alt-text][Snap Fit 2]
![alt-text][Snap Fit 3]

### Good Design Practice

![alt-text][Good Design Practice 1]
![alt-text][Good Design Practice 2]

### Calculations [[3]]

![alt-text][Snap Fit Calculations 1]
![alt-text][Snap Fit Calculations 2]


### Rules of Thumb
* The thickness of the optimal snapfit arm decreases linearly to 30% of the original cross-sectional area. The strain in the
outer fibers is uniform throughout the length
of the cantilever

## Enclosure
### Design

![alt-text][Structure Design 1]
![alt-text][Structure Design 2]
![alt-text][Structure Design 3]

### Rules of Thumb
* **A minimum wall thickness of 2 mm, 0.5 mm tolerance around internal components and ± 0.25 mm for clearance/bite holes are good start points to consider when designing a 3D printed enclosure.**
	* Good results have been obtained by reducing the thickness (h) of the cantilever linearly so that its value at the end of the hook is equal to one-half the value at the root;


## Sliding Joint

![alt-text][Sliding Joint 1]
![alt-text][Sliding Joint 3] 
![alt-text][Sliding Joint 2] 
![alt-text][Sliding Joint Printing]


[1]: https://community.smartthings.com/t/how-to-seal-weatherproof-a-sensor-device-anything-better-than-silicone/93594/12
[2]: https://www.fabbaloo.com/blog/2017/10/19/waterproofing-your-3d-prints
[3]: http://fab.cba.mit.edu/classes/S62.12/people/vernelle.noel/Plastic_Snap_fit_design.pdf

[Good Design Practice 1]: https://i.ibb.co/Rg6QJYH/Good-Design-Practice-1.png
[Good Design Practice 2]: https://i.ibb.co/Q9vY8bf/Good-Design-Practice-2.png
[Structure Design 1]: https://i.ibb.co/Th9M2Fx/Structure-Design-1.png
[Structure Design 2]: https://i.ibb.co/JKKD0gt/Structure-Design-2.png
[Structure Design 3]: https://i.ibb.co/d6Cg48Z/Structure-Design-3.png
[Printing Tip]: https://i.ibb.co/9Tr8CFp/Snap-Fit-Printing.png
[Snap Fit Calculations 1]: https://i.ibb.co/BPs4d51/Snap-Fit-Calculations-1.png
[Snap Fit Calculations 2]: https://i.ibb.co/9yqybMs/Snap-Fit-Calculations-2.png

[Cantilever Snap Joint]: https://i.ibb.co/jwxTJFH/Cantilever-snap-joint.png
[Sliding Joint 1]: https://i.ibb.co/hfFjyv6/Sliding-Joint-1.png
[Sliding Joint 2]: https://i.ibb.co/m8p952w/Sliding-Joint-2.png
[Sliding Joint 3]: https://i.ibb.co/4tsKrbx/Sliding-Joint-3.png
[Sliding Joint Printing]: https://i.ibb.co/VYc5Kpz/Sliding-Joint-Printing.png
[Snap Fit 1]: https://i.ibb.co/M8k8ZJ2/Snap-Fit-1.png
[Snap Fit 2]: https://i.ibb.co/ZVpNhQV/Snap-Fit-2.png
[Snap Fit 3]: https://i.ibb.co/mNDRkPG/Snap-Fit-3.png