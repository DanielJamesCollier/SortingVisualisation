#include <iostream>
#include <string>
#include <iostream>
#include <array>
#include <chrono>
#include <cstddef>
#include <thread>
#include <cmath>
#include <vector>
#include <limits>

#include "sdl_module.hpp"

using namespace std::chrono_literals;

constexpr auto width = 600;
constexpr auto height = 300;
constexpr int max_height = 500;
constexpr std::size_t size = 50;

sdl_module sdl("sdl_module", width, height);

std::vector<std::size_t> update_list;

//----------------------
template<typename T>
struct value {
    value()
    : percent{1.0}
    , actual{0}
    , render{0}
    {
        // empty
    }
    
    value(T value)
    : percent{1.0}
    , actual{(double)value}
    , render{(double)value}
    {
        // empty
    }
    double percent;
    double actual;
    double render;
};

std::array<value<int>, size> array;

//----------------------
float
clamp(float x, float lowerlimit, float upperlimit) {
    if (x < lowerlimit) x = lowerlimit;
    if (x > upperlimit) x = upperlimit;
    return x;
}

//------------------------------------------------------------
template<typename T>
constexpr T
map_0B_0D(T value, T b, T d) {
    return value * (d / b);
}

//------------------------------------------------------------
template<typename T>
constexpr T
map(T value, T a, T b, T c, T d) {
    return (value - a) * (d - c) / (b - a) + c;
}

//------------------------------------------------------------
template <typename T>
constexpr T
lerp(T v0, T v1, double t) {
    return (T(1) - t) * v0 + t * v1;
}

//----------------------
template<typename T>
void
swap(T &a, T &b, std::size_t a_index, std::size_t b_index) {
    
    T c = a;
    a = b;
    b = c;
    
    update_list.push_back(a_index);
    update_list.push_back(b_index);
    
    double t = 0;
    
    while (update_list.size() > 0) {
   
        
        for (int i = 0; i < update_list.size(); i++) {
            if (array[update_list[i]].render < array[update_list[i]].actual) {
                array[update_list[i]].render++;
            } else if (array[update_list[i]].render > array[update_list[i]].actual) {
                array[update_list[i]].render--;
            } else {
                update_list.erase(update_list.begin() + i);
            }
        }
        
    
        SDL_SetRenderDrawColor(sdl.m_renderer, 33, 4, 61, 255);
        SDL_RenderClear(sdl.m_renderer);
        
        SDL_Rect rect;
        rect.y = sdl.m_renderer_height;
        rect.w = sdl.m_renderer_width / size;
        
       
        
        // render
        for (int i = 0; i < size; i++) {
            rect.x = i * rect.w;
            rect.h = -array[i].render;
            
            
            unsigned int red = map((unsigned int)rect.h, (unsigned int)0, (unsigned int)sdl.m_renderer_height, (unsigned int)100, (unsigned int)255);
            unsigned int blue = map((unsigned int)i, (unsigned int)0, (unsigned int)size, (unsigned int)100, (unsigned int)255);
            SDL_SetRenderDrawColor(sdl.m_renderer, red, 0, blue, 255);
            SDL_RenderFillRect(sdl.m_renderer, &rect);
        }
        
        // render selection outline
        SDL_Rect selection_a;
        SDL_Rect selection_b;
        
        selection_a.x = a_index * rect.w;
        selection_a.y = sdl.m_renderer_height;
        selection_a.w = sdl.m_renderer_width / size;
        selection_a.h = -array[a_index].render;
        
        selection_b.x = b_index * rect.w;
        selection_b.y = sdl.m_renderer_height;
        selection_b.w = sdl.m_renderer_width / size;
        selection_b.h = -array[b_index].render;
        
        SDL_SetRenderDrawColor(sdl.m_renderer, 255, 0, 0, 255);
        SDL_RenderDrawRect(sdl.m_renderer, &selection_a);
        SDL_RenderDrawRect(sdl.m_renderer, &selection_b);
        
        
        SDL_RenderPresent(sdl.m_renderer);
    }
}

//----------------------
template<std::size_t size>
void
selection_sort(std::array<value<int>, size> &array) {
    for (decltype(size) i = 0; i < size; ++i) {
        
        decltype(size) lowest_index = i;
        
        for (decltype(size) j = i + 1; j < size; ++j) {
            if (array[lowest_index].actual > array[j].actual) {
                lowest_index = j;
            }
            // maybe draw something here so we can visualise the search?
        }
      
        swap(array[i].actual, array[lowest_index].actual, lowest_index, i);
    }
}

//----------------------
template<std::size_t size>
void
fill_array_with_rand(std::array<value<int>, size> &array) {
    for (auto &i : array) {
        i = value<int>(rand() % max_height);
    }
}

//----------------------
template<std::size_t size>
bool
is_sorted(std::array<int, size> &array) {
    for (decltype(size) i = 0; i < size - 1; ++i) {
        if (array[i] > array[i + 1]) {
            return false;
        }
    }
    return true;
}

//----------------------
template<std::size_t size>
void
bubble_sort(std::array<value<int>, size> &array) {
    SDL_SetRenderDrawColor(sdl.m_renderer, 0, 0, 0, 255);
    for (decltype(size) i = 0; i < size - 1; ++i) {
        for (decltype(size) j = 0; j < size - i - 1; ++j) {
            if (array[j].actual > array[j + 1].actual) {
                swap(array[j].actual, array[j + 1].actual, j, j + 1);
            }
        }
    }
}

int
main() {
    std::cout << "hello sailor!\n";
    
    fill_array_with_rand(array);
    
    std::cout << " requeted res: " << width << " x " << height << '\n';
    std::cout << "renderer res: " << sdl.m_renderer_width << " x " << sdl.m_renderer_height << '\n';
    
    SDL_SetRenderDrawColor(sdl.m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdl.m_renderer);
    
    SDL_Rect rect;
    rect.y = sdl.m_renderer_height;
    rect.w = sdl.m_renderer_width / size;
    
    
    // render
    for (int i = 0; i < size; i++) {
        rect.x = i * rect.w;
        rect.h = -array[i].render;
        
        
        unsigned int red = map((unsigned int)rect.h, (unsigned int)0, (unsigned int)sdl.m_renderer_height, (unsigned int)100, (unsigned int)255);
        unsigned int blue = map((unsigned int)i, (unsigned int)0, (unsigned int)size, (unsigned int)100, (unsigned int)255);
        SDL_SetRenderDrawColor(sdl.m_renderer, red, 0, blue, 255);
        SDL_RenderFillRect(sdl.m_renderer, &rect);
    }
    
    SDL_RenderPresent(sdl.m_renderer);
    
    selection_sort(array);
    fill_array_with_rand(array);
    bubble_sort(array);
    return 0;
}
