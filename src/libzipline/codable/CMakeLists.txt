if(PROJECT_TESTING)
    target_sources(zipline.test PRIVATE
        bool.test.cpp
        coder.test.hpp
        enum.test.cpp
        integral.test.cpp
        object.test.cpp
        optional.test.cpp
        pair.test.cpp
        string.test.cpp
        time_point.test.cpp
        vector.test.cpp
    )
endif()
