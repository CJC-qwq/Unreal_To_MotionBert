import unreal


MAP_PATH = "/Game/ThirdPerson/Lvl_ThirdPerson"
GAME_MODE_PATH = "/Script/MotionBERT_UE.MotionBERTMocapPreviewGameMode"


def main():
    world = unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH)
    if not world:
        raise RuntimeError(f"Failed to load map: {MAP_PATH}")

    world_settings = world.get_world_settings()
    if not world_settings:
        raise RuntimeError("Failed to access world settings.")

    game_mode_class = unreal.load_class(None, GAME_MODE_PATH)
    if not game_mode_class:
        raise RuntimeError(f"Failed to load game mode class: {GAME_MODE_PATH}")

    world_settings.set_editor_property("default_game_mode", game_mode_class)

    saved = unreal.EditorLoadingAndSavingUtils.save_current_level()
    if not saved:
        raise RuntimeError("Failed to save current level after updating game mode override.")

    unreal.log(f"Updated {MAP_PATH} default_game_mode -> {GAME_MODE_PATH}")


if __name__ == "__main__":
    main()
