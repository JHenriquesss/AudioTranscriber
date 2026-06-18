cmake_minimum_required(VERSION 3.24)

add_executable(phase05_unit_tests
    test_result.cpp
    test_error.cpp
    test_cancellation_token.cpp
    test_time.cpp
    test_architecture_guard.cpp
    test_subtitle_timestamp.cpp
    test_transcript_validation.cpp
    test_export_golden.cpp
    test_export_service.cpp
    test_app_paths.cpp
    test_app_settings.cpp
    test_job_status.cpp
    test_transcription_job.cpp
    test_job_manager.cpp
    test_logger.cpp
    test_process_runner.cpp
    test_ffmpeg_args.cpp
    test_audio_probe.cpp
    test_audio_normalizer.cpp
    test_audio_integration.cpp
    test_whisper_options_mapper.cpp
    test_whisper_model.cpp
    test_transcript_post_processor.cpp
    test_transcription_pipeline.cpp
)

target_link_libraries(phase05_unit_tests
    PRIVATE
        offline_transcriber_app
        offline_transcriber_export
        offline_transcriber_audio
        offline_transcriber_transcription
        offline_transcriber_whisper_engine
        Catch2::Catch2WithMain
)

target_compile_definitions(phase05_unit_tests
    PRIVATE
        PROJECT_SOURCE_ROOT="${CMAKE_SOURCE_DIR}"
)

include(Catch)
catch_discover_tests(phase05_unit_tests)
