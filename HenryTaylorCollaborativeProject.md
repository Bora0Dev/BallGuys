## Henry Taylor 2206046
## Collaborative Project
### Ball Guys: Multiplayer Physics-Based Gameplay Prototype
________________________________________
### 1. Introduction
**Ball Guys** originated from an early networking test designed to validate multiplayer movement and replication in Unreal Engine. This initial technical experiment involved a host and client each controlling a place-holder sphere within a shared environment. Through iterative experimentation and testing, this prototype evolved organically into a playable multiplayer experience.

The project was developed as a collaborative multiplayer prototype in which players control a physics-based rolling ball character. Its core aim was to integrate contributions from collaborators into a cohesive gameplay experience while demonstrating iterative development, player-driven interaction, event-driven systems, audio feedback, and user interface design.

Development was carried out in Unreal Engine 5.6.1 using a server-authoritative multiplayer architecture. My primary role within the collaboration focused on the implementation of core gameplay systems, including player movement, physics interaction, input handling, networking logic, user interface integration, and quality assurance testing. These systems were designed to support and respond to assets and mechanics introduced by collaborators, such as environmental hazards, jump pads, and level layouts.

This essay documents both the technical implementation and the iterative problem-solving processes involved in developing Ball Guys. Particular emphasis is placed on how collaborator feedback influenced system design decisions, how emergent gameplay issues were addressed through iteration, and how event-driven systems were used to connect gameplay mechanics with audio and user interface feedback. The essay concludes with a reflection on lessons learned and potential areas for future improvement.
________________________________________
### 2. Project Context and Collaboration
**Ball Guys** was conceived as a collaborative prototype intended to combine programming, design, and gameplay experimentation. Collaborators contributed level design elements, environmental mechanics, networking capabilities, session management and visual assets, which were then integrated into the playable experience. Rather than treating these contributions as isolated features, a key objective was to ensure that collaborator content meaningfully interacted with the core gameplay systems.

One example of this integration was the introduction of jump pads, which applied vertical impulses to the player character. While conceptually simple, these pads significantly affected player movement and revealed limitations within the original movement system. Similarly, kill volumes, boost pads, and environmental obstacles introduced by collaborators required careful consideration of how physics-based movement behaved under edge-case conditions. This required gameplay systems to remain flexible and responsive, as collaborator features were often introduced before their full gameplay implications were known.

The collaborative process highlighted challenges common to team-based development, such as coordinating multiplayer testing and integrating interdependent systems. Multiplayer testing, in particular, revealed issues that were not apparent during single-player or Play-In-Editor testing. These challenges informed both technical decisions and workflow improvements over the course of the project.
________________________________________
### 3. Core Gameplay Systems
#### 3.1 Physics-Based Player Movement
At the heart of Ball Guys is a physics-simulated pawn that uses torque-based movement rather than traditional character locomotion. This approach was chosen to reinforce the game’s playful, momentum-driven feel and to allow environmental forces to meaningfully influence player behaviour.

Player input is translated into torque applied to a spherical mesh, with movement direction determined relative to the player’s camera orientation. This ensured intuitive control, allowing players to steer the ball based on where they are looking rather than relying on world-aligned axes. Movement logic is executed authoritatively on the server, with physics replication used to synchronise movement across clients.
#### 3.2 Jump and Boost Mechanics
Jumping is implemented as a physics impulse applied upward to the ball, gated by a grounded check to prevent infinite mid-air jumps. A boost mechanic was added as an additional player-driven interaction, temporarily increasing both movement torque and collision impulse strength. The boost system includes a duration and cooldown, both of which are tracked on the server and replicated to clients for user interface feedback.

This mechanic introduced an additional layer of decision-making, encouraging players to time boosts strategically rather than using them continuously. The boost system also served as a useful demonstration of event-driven UI updates and replicated gameplay state.
#### 3.3 Multiplayer Architecture
All core movement and interaction logic is executed on the server to maintain authoritative control and prevent desynchronisation or cheating. Clients send input data to the server via RPCs, while physics results and replicated variables are used to update client-side visuals and UI. This architecture required careful separation between client-only input handling and server-only state updates, particularly when debugging issues related to latency and controller input.
#### 3.4 Player Lives Versus Player Score
During early design discussions, consideration was given to implementing a traditional score-based system to reward players for successful collisions or knockouts. However, practical testing quickly revealed that scoring in a physics-driven multiplayer environment introduced significant ambiguity. In scenarios where multiple players collided simultaneously, it became unclear how points should be assigned. For example, if three players collided and one or more were knocked out of the play area, attributing responsibility in a fair and deterministic way proved non-trivial.

Because collisions are resolved by the physics system rather than discrete player actions, outcomes were often emergent rather than clearly attributable to a single player. Introducing a scoring system under these conditions risked producing results that felt arbitrary or unfair, particularly in a competitive multiplayer context.

To address this, a lives-based system was adopted instead. Players lose a life when they are eliminated from the play area, and the round continues until a defined end condition is reached. This approach provided clear, unambiguous feedback to the player while aligning naturally with the chaotic, physics-driven nature of the gameplay.

From a technical perspective, the lives system was also simpler to implement and replicate reliably across the network, reducing the risk of desynchronisation or disputed outcomes. More importantly, it reinforced the intended focus on moment-to-moment survival and positioning rather than numerical optimisation. This decision reflects a deliberate trade-off between mechanical complexity and gameplay clarity, prioritising player experience and fairness over unnecessary system overhead.
________________________________________
### 4. Iteration and Problem Solving
#### 4.1 Jump Pad Trapping and Air Control
One of the most significant gameplay issues emerged following the integration of jump pads contributed by a collaborator. These pads applied a strong upward impulse when the player overlapped them. Under certain conditions, players could become trapped in a vertical loop, repeatedly re-triggering the pad without ever returning to the ground.

The original movement system only allowed torque-based control when the player was grounded. As a result, once airborne, players had no ability to influence their horizontal movement and could not escape the jump pad’s vertical force. This issue was identified through collaborative playtesting and highlighted a mismatch between intended design and actual player experience.

An initial solution involved enabling full air control by applying torque while airborne. While technically effective, this approach removed much of the intended weight and momentum from the movement system, resulting in movement that felt floaty and less physically grounded.

Through further iteration, a more subtle solution was implemented. Instead of applying torque mid-air, a small lateral force was applied while airborne, allowing players limited steering capability without overriding the core physics-based movement. This preserved the sense of momentum and weight while giving players just enough agency to escape vertical traps.

This iteration demonstrated the importance of balancing mechanical control with game feel. Rather than seeking a purely functional solution, the final implementation prioritised player experience, ensuring that the mechanic felt fair and intuitive without undermining the game’s core movement identity.
#### 4.2 Persistence of Axis Inversion UI After Player Death and Respawn
A further iteration challenge emerged around player camera inversion settings and their associated user interface feedback. The project allowed players to invert the X and Y camera axes during live gameplay using input actions rather than a traditional pause menu. While the inversion itself functioned correctly, playtesting revealed that the visual state of the on-screen checkboxes did not update correctly after the player died and respawned.

The underlying cause of this issue was the lifecycle of the player pawn. Upon death, the pawn was destroyed and a new pawn instance was spawned and possessed by the PlayerController. While the inversion booleans themselves were still toggled correctly during gameplay, the UI widget retained references to the original pawn instance, resulting in stale or misleading visual feedback.

This issue was resolved by persisting the inversion preferences at the GameInstance level rather than the pawn level. The GameInstance was used as a lightweight, session-persistent data store for player preferences. On pawn possession, the newly spawned pawn read the stored inversion values from the GameInstance and applied them immediately, ensuring behavioural consistency.

In parallel, the UI widget was updated to dynamically bind to the currently possessed pawn rather than assuming a static reference. By rebinding the widget during the PlayerController’s possession event, the UI was able to correctly reflect the inversion state after every respawn. This solution ensured that players received accurate, real-time feedback and reinforced the importance of separating persistent player preferences from transient gameplay actors in Unreal Engine.
________________________________________
#### 4.3 Resolving Host and Client Input Conflicts in Packaged Multiplayer Builds
Another significant issue arose during multiplayer testing of packaged builds, specifically relating to controller input. While gamepad input functioned correctly during Play-In-Editor testing, inconsistent behaviour was observed in standalone and networked builds. In some cases, controller input would work on first launch but fail on subsequent runs, particularly when testing hosted games over the internet.

Through systematic debugging and elimination of variables, it was discovered that the issue was not related to Unreal Engine’s Enhanced Input system itself, but rather to an external conflict caused by Steam Input. When Steam Input was enabled, it intercepted controller signals before they reached Unreal Engine’s XInput and Enhanced Input Mapping Context systems, resulting in lost or duplicated input events.

The resolution involved disabling Steam Input on a per-game basis and documenting this requirement for players. Once Steam Input was disabled, controller behaviour became consistent across hosts and clients, both in local and online testing scenarios. Importantly, this issue could not be resolved purely through code changes, highlighting the necessity of understanding platform-level input handling in addition to engine-level systems.

This process reinforced the importance of testing packaged builds early and under real deployment conditions. It also demonstrated that some multiplayer issues originate outside the engine itself, requiring a broader troubleshooting mindset that considers operating system, platform services, and third-party middleware.
________________________________________
#### 4.4 Systematic Debugging and Iterative Troubleshooting
Consistent and deliberate debugging was a central practice throughout the development of Ball Guys. Given the complexity of physics-driven multiplayer systems, issues often arose that were not immediately visible through static inspection of code or Blueprints.

Debug output was used extensively to visualise real-time values such as velocity magnitude, collision impulse strength, grounded state, boost cooldown timers, and input activation. This allowed gameplay behaviour to be observed empirically rather than inferred, significantly reducing guesswork during tuning and troubleshooting.

In several cases, issues that initially appeared to be logic errors were revealed to be execution order problems or lifecycle mismatches between actors, widgets, and controllers. The distinction between data flow and execution flow within Unreal Engine became particularly important when debugging Blueprint-based systems, such as UI updates and audio triggering.

This iterative debugging approach proved especially valuable in a collaborative environment, where changes introduced by one system could have unintended side effects elsewhere. By treating debugging as an ongoing development tool rather than a final corrective step, the project maintained stability despite frequent iteration and integration of new features.

________________________________________
### 5. Physics-Driven Audio Feedback (BallPawn)
A key area of development within the project was the implementation of responsive audio feedback for the player-controlled BallPawn. As the core mechanic of the game relies on physics-based movement, audio was treated as a critical component for reinforcing player actions such as rolling, bouncing, and high-impact collisions.

Rather than relying on animation-driven audio cues, the system was implemented using real-time physics data. Rolling audio was driven by the magnitude of the pawn’s linear velocity, allowing sound intensity and playback state to reflect the ball’s actual movement speed. A hysteresis approach was used, with separate start and stop thresholds, to prevent rapid audio toggling at low velocities. This ensured stable playback and avoided distracting artefacts.

To address an early issue where rolling audio continued while the ball was airborne, a downward line trace was introduced to determine whether the pawn was grounded. This grounded check decoupled audio playback from pure velocity alone, aligning the sound behaviour with player expectations and real-world physical logic. This solution also highlighted the importance of separating data evaluation (velocity and trace results) from execution flow within Unreal Engine’s Blueprint system.

Collision audio was implemented using the OnComponentHit event, with impact strength calculated from the magnitude of the physics Normal Impulse. Two categories of collision feedback were defined: lighter impacts triggered a bounce sound, while heavier impacts triggered a distinct hit sound. Initial testing revealed that physics contact resolution could cause repeated collision events during continuous contact, resulting in excessive audio playback. This was resolved through the introduction of both impulse thresholds and a short cooldown timer, ensuring that sounds were only played for meaningful impacts.

To further improve perceived quality and reduce repetition, subtle pitch variation was applied to both bounce and hit sounds using Sound Cues. This allowed variation to be handled at the audio asset level rather than increasing Blueprint complexity, maintaining a clean separation between gameplay logic and audio design.

This process required significant iteration and debugging, particularly in understanding Blueprint execution flow versus data flow, as well as interpreting raw physics values for tuning thresholds. Debug output was used extensively to observe real-time impulse and velocity values, allowing parameters to be adjusted empirically rather than through guesswork. Through this iteration, the final system became both robust and easily tuneable, with minimal performance overhead and no reliance on network replication.
________________________________________
### 6. Event-Driven Systems and User Interface
User interface elements in Ball Guys were implemented to provide clear, responsive feedback without overwhelming the player. Two primary UI components were developed: a boost cooldown indicator and an axis inversion display that allowed players to adjust camera preferences during gameplay without entering a pause menu.

The boost UI was driven by replicated gameplay variables, updating dynamically based on boost duration and cooldown state. Because the player pawn is destroyed and respawned during gameplay, special care was taken to ensure that the UI correctly rebound to the newly possessed pawn. This was handled via event-driven logic in the PlayerController, ensuring that each client only ever saw their own UI instance.

The axis inversion UI demonstrated live feedback, allowing players to toggle camera preferences and immediately see the results reflected both in gameplay and on-screen indicators. Player preferences were stored persistently using the GameInstance, ensuring that settings persisted across respawns.

These systems collectively demonstrated the use of event-driven architecture to connect gameplay state, user input, and UI feedback in a multiplayer-safe manner.
________________________________________
### 7. Reflection and Learning Outcomes
Working within a collaborative environment reinforced the need to design systems that are robust, adaptable, and resilient to change. As new gameplay elements were introduced, such as jump pads, boost mechanics, and kill volumes, existing systems were repeatedly stress-tested in ways that could not have been fully anticipated during initial implementation. This emphasised the importance of building flexible core systems and revisiting earlier decisions as the project evolved.

From a technical perspective, the project significantly deepened my understanding of Unreal Engine’s physics simulation, networking model, and event-driven architecture. Implementing server-authoritative movement, replicated gameplay state, and responsive UI systems required careful separation of concerns between client-side input, server-side logic, and presentation. The air control iteration, in particular, reinforced the principle that player agency should enhance, rather than undermine, the underlying physical rules of the game.

The project also strengthened my debugging and problem-solving skills. Solutions were derived primarily through iterative testing, runtime debugging, and direct observation of Unreal Engine’s physics and actor lifecycle behaviour rather than reliance on prescriptive documentation. Systematic use of runtime debug output, incremental testing, and isolation of variables proved essential when resolving complex issues related to physics behaviour, UI lifecycle management, and platform-level input conflicts. These experiences highlighted the importance of treating debugging as an ongoing development practice rather than a final corrective step.

If revisiting the project, additional time would be allocated to earlier multiplayer testing and further refinement of player feedback and onboarding systems. Nonetheless, Ball Guys successfully demonstrates a cohesive integration of collaborator contributions, iterative problem-solving, and responsive gameplay systems, and represents a meaningful step forward in my technical and professional development.
________________________________________
### 8. Conclusion
Ball Guys represents a successful collaborative prototype that integrates physics-based movement, multiplayer architecture, audio feedback, and user interface systems into a cohesive gameplay experience. Through iterative development and close integration of collaborator contributions, the project demonstrates both technical competence and reflective practice.

The challenges encountered throughout development provided valuable learning opportunities, particularly in balancing mechanical control, player experience, and collaborative workflows. Overall, the project meets the stated requirements and serves as a strong foundation for future development and professional practice.
________________________________________
### 9. Sample Gameplay

https://github.com/user-attachments/assets/d61c17b8-e852-45f5-ba58-f406d5a236a1


