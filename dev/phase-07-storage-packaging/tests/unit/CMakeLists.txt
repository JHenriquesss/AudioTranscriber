cmake_minimum_required(VERSION 3.24)

add_executable(phase07_unit_tests
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
    test_transcription_request_validation.cpp
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
    test_database.cpp
    test_job_repository.cpp
    test_transcript_repository.cpp
    test_model_repository.cpp
    test_job_history_coordinator.cpp
    test_model_catalog.cpp
)

target_link_libraries(phase07_unit_tests
    PRIVATE
        offline_transcriber_app
        offline_transcriber_storage
        offline_transcriber_export
        offline_transcriber_audio
        offline_transcriber_transcription
        offline_transcriber_whisper_engine
        Catch2::Catch2WithMain
)

target_compile_definitions(phase07_unit_tests
    PRIVATE
        PROJECT_SOURCE_ROOT="${CMAKE_SOURCE_DIR}"
)

include(Catch)
catch_discover_tests(phase07_unit_tests)
