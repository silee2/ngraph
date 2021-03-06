//*****************************************************************************
// Copyright 2017-2020 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*****************************************************************************

#include <iostream>
#include <tuple>

int count()
{
    static int counter = 0;
    return counter++;
}

std::tuple<int, int> stateless_count(int counter)
{
    return std::tuple<int, int>(counter, counter + 1);
}

int main()
{
    std::cout << "State: " << count() << ", " << count() << std::endl;

    std::cout << "Stateless: ";
    int counter = 0;
    {
        auto r(stateless_count(counter));
        counter = std::get<1>(r);
        std::cout << std::get<0>(r);
    }
    std::cout << ", ";
    {
        auto r(stateless_count(counter));
        counter = std::get<1>(r);
        std::cout << std::get<0>(r);
    }
    std::cout << std::endl;

    return 0;
}
