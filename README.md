# BallGuys


## Technical Commentary

### Networking Implementation and Configuration
The networking for *BallGuys* is built upon **Unreal Engine 5's** robust client-server architecture, leveraging the **Online Subsystem Steam** for session management and connectivity. The project is configured to use the `SteamSocketsNetDriver`, which provides a reliable transport layer over Steam's relay network, ensuring stable connections even in peer-to-peer (P2P) scenarios.

The configuration is defined in `DefaultEngine.ini`, where the `OnlineSubsystemSteam` is enabled and set as the default platform service. The Steam App ID is set to `480` (Spacewar), a standard practice for development and testing, allowing the game to utilize Steam's matchmaking and lobby features without a published application ID.

### Player Connection and Replication
Players connect to the game through a listen-server model. One player acts as the Host, creating a session via the Steam Online Subsystem. Other players (Clients) search for and join this session. The connection flow is handled seamlessly by the engine's underlying network drivers, with Steam handling the NAT traversal and handshake processes.

**Replication** is central to the multiplayer experience, ensuring all players perceive the same game state. The project utilizes Unreal's **Actor Replication** system with a focus on property replication over RPCs for continuous state synchronization:
*   **Property Replication**: Critical variables are synchronized from the Server to Clients using the `DOREPLIFETIME` macro within `GetLifetimeReplicatedProps`. 
    *   **GameState (`ABallGuysGameState`)**: Manages the global flow of the match. Variables such as `TimeRemaining` and `CurrentGamePhase` are replicated to ensure all clients display the correct timer and transition between lobby, gameplay, and post-match states simultaneously.
    *   **PlayerState (`ABallGuysPlayerState`)**: Handles individual player data that must persist even if the pawn is destroyed. `CurrentLives` and `bIsReady` are replicated here, allowing the UI to update player status and scoreboards dynamically across all clients.
    *   **Pawn (`ABallPawn`)**: Controls the physical representation of the player. `bIsBoosting`, `BoostTimeRemaining`, and `CooldownTimeRemaining` are replicated to synchronize visual effects (like boost trails) and gameplay mechanics, ensuring that when one player boosts, others see the acceleration and particle effects in real-time.
*   **RPCs (Remote Procedure Calls)**: The analysis of the codebase indicates a heavy reliance on property replication for state management. This approach minimizes network bandwidth usage by only sending updates when values change, rather than firing frequent RPCs for continuous actions. This is particularly effective for the physics-based movement of the "BallGuys," where the server acts as the authoritative source for position and velocity, and clients interpolate the results.

### Tools, Frameworks, and APIs
The development relied on a suite of industry-standard tools and frameworks:
*   **Unreal Engine 5**: Provided the core game engine, physics simulation, and networking authority.
*   **Online Subsystem Steam API**: Used for handling user authentication, lobbies, and matchmaking.
*   **SteamSockets**: Utilized for the low-level network transport layer.
*   **Git**: Employed for version control, allowing for branch management and code merging.

### Collaboration and Playtesting
Collaboration during development was facilitated through **Git** for source control, enabling the team to work on different features simultaneously and merge changes.

Playtesting was conducted using the Steam App ID 480. This allowed team members to easily host and join games directly through their Steam friends list or via a development server browser. This approach removed the need for setting up dedicated servers during the early stages, allowing for rapid iteration and testing of network latency and replication logic in a real-world P2P environment.

## Bora
**Bora0Dev** has been instrumental in the core development and maintenance of *BallGuys*. Key contributions include:
*   **Core Gameplay Mechanics**: Refactored the movement input system to utilize control rotation, significantly improving the control scheme and responsiveness for the "BallGuys" character.
*   **Multiplayer Plugin**: Implemented and configured the Multiplayer Plugin, enabling robust networking capabilities and session management.
*   **HUD Implementation**: Designed and implemented the Heads-Up Display (HUD) to provide real-time feedback to players, including scoreboards and status updates.
*   **Project Initialization**: Established the initial project structure and repository, laying the groundwork for development.

## Demo

[![Ball Guys - Steam Multiplayer Integration](https://img.youtube.com/vi/Etx-swdNkSw/0.jpg)](https://www.youtube.com/watch?v=Etx-swdNkSw)
