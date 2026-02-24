# Beat-Synced Game Sample

Rhythm game with beat detection, visual feedback, and score system synchronized to music.

## Overview

This sample demonstrates:
- **Real-time beat detection** using `OnsetDetector`
- **Synchronization** of game events to music beats
- **Visual feedback** (particle effects, UI feedback)
- **Score system** based on timing accuracy
- **Game state management** tied to audio events

## What You'll Learn

- How to use `OnsetDetector` for beat detection
- How to synchronize game mechanics to audio events
- How to implement visual feedback for beat hits
- How to create satisfying game feel with audio-visual synchronization
- Best practices for rhythm game development

## Quick Start

1. Open the sample project in Unity 2021.3 or newer
2. Open the `BeatSyncedGame` scene
3. Attach an AudioSource with music
4. The `BeatDetector` component starts detecting beats
5. Click game elements on beat to score points
6. Watch the score update based on timing accuracy

## Files Included

- **BeatDetector.cs** - Onset detection and beat event system
- **BeatVisualizer.cs** - Visual feedback on beat detection
- **GameController.cs** - Game state and scoring
- **UIManager.cs** - Score display and feedback

## Full Documentation

For comprehensive details and implementation guide, see:
[docs/ExampleProjects.md](../../docs/ExampleProjects.md#example-3-beat-synced-game)

## Features

- **Configurable Detection**: Adjust threshold and minimum interval
- **Visual Feedback**: Particle effects and UI flashes on beat
- **Scoring System**: Points based on hit timing (early/on-beat/late)
- **Combo Multiplier**: Build combos for higher scores
- **Difficulty Levels**: Easy/Normal/Hard detection thresholds

## Next Steps

- Implement different game mechanics (tap, hold, slide)
- Add visual effects synchronized to different frequency bands
- Create different songs with varying difficulty
- Implement leaderboard system
- Add haptic feedback for beat detection

## Tips for Best Results

- Use music with clear, strong beats
- Adjust the detection threshold for different audio styles
- Test on target device to account for audio latency
- Consider platform-specific audio buffering delays

---

**Let's make some rhythm games! 🎮🎵**
