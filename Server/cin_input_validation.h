#pragma once
#pragma once
#include <iostream>

namespace tools
{
    template<typename valueType, typename isTheValueValidFunction>
    valueType GetValidValueFromUser(std::string message, isTheValueValidFunction isTheValueValid)
    {
        valueType value;
        bool isInvalid = true;

        while (isInvalid)
        {
            std::cout << message;
            std::cin >> value;
            bool isStreamFail = std::cin.fail();
            isInvalid = isStreamFail || !isTheValueValid(value) || std::cin.peek() != '\n';
            if (isInvalid)
            {
                if (isStreamFail)
                    std::cin.clear();

                std::cout << "The value is not valid, please try again." << std::endl;
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        }

        system("clear");
        return value;
    }
}