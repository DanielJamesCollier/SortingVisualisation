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
#include <algorithm>
#include <time.h>
#include "sdl_module.hpp"

using namespace std::chrono_literals;

constexpr auto width = 1000;
constexpr auto height = 600;
constexpr std::size_t size = 100;

sdl_module sdl("sdl_module", width, height);

struct selection {
    selection(std::size_t selected, int r, int g, int b)
    :   selected{selected}
    ,   r{r}
    ,   g{g}
    ,   b{b}{}
    
    std::size_t selected;
    int r;
    int g;
    int b;
};

std::vector<std::size_t> update_list;
std::vector<selection> selected;
SDL_Rect rect;

//----------------------
template<typename T>
struct value {
    value()
    : last{0}
    , actual{0}
    , render{0}
    {
        // empty
    }
    
    value(T value)
    : last{(double)value}
    , actual{(double)value}
    , render{(double)value}
    {
        // empty
    }
    double last;
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

//------------------------------------------------------------
void
select(std::size_t i, int r, int g, int b) {
    selected.emplace_back(i, r, g, b);
}

//------------------------------------------------------------
void
begin_draw() {
    SDL_SetRenderDrawColor(sdl.m_renderer, 0, 0, 61, 255);
    SDL_RenderClear(sdl.m_renderer);
}

//------------------------------------------------------------
void
end_draw() {
    SDL_RenderPresent(sdl.m_renderer);
}

//------------------------------------------------------------
void
draw_all_bars() {
    for (int i = 0; i < size; i++) {
        rect.x = (i * rect.w) + 1;
        rect.h = -array[i].render;
        
        SDL_Rect temp = rect;
        temp.w -= 1;
   /*
        float mini = 0, maxi = sdl.m_renderer_height;
        float ratio = 2 * (array[i].render - mini) / (maxi - mini);
        int b = (int)std::max(0.0, 255.0 * (1.0 - ratio));
        int r = (int)std::max(0.0, 255.0 * (ratio - 1.0));
        int g = 255 - b - r;
        */
        unsigned int r = map((unsigned int)rect.h, (unsigned int)0, (unsigned int)sdl.m_renderer_height, (unsigned int)100, (unsigned int)255);
        unsigned int b = map((unsigned int)i, (unsigned int)0, (unsigned int)size, (unsigned int)150, (unsigned int)255);
        unsigned int g = 0;

        SDL_SetRenderDrawColor(sdl.m_renderer, 0, g, b, 255);
        SDL_RenderFillRect(sdl.m_renderer, &temp);
    }
}

//------------------------------------------------------------
void
draw_selected() {
    begin_draw();
    {
        draw_all_bars();
        
        SDL_Rect temp = rect;
       // temp.w -= 1;
        
        for (auto& i : selected) {
            temp.x = i.selected * rect.w;
            temp.h = -array[i.selected].render;
            SDL_SetRenderDrawColor(sdl.m_renderer, i.r, i.g, i.b, 255);
            SDL_RenderDrawRect(sdl.m_renderer, &temp);
        }
    }
    end_draw();
}

//----------------------
template<typename T>
void // TODO. can remove the ref args and just use the size_t
swap(T &a, T &b, std::size_t a_index, std::size_t b_index) {
    T c = a;
    a = b;
    b = c;
    
    update_list.push_back(a_index);
    update_list.push_back(b_index);
    
    array[a_index].last = array[a_index].render;
    array[b_index].last = array[b_index].render;
    
    double j = 0;
    
    while (update_list.size() > 0) {
        j += 0.02;
        
        for (int i = 0; i < update_list.size(); i++) {
          if (array[update_list[i]].render != array[update_list[i]].actual) {
              array[update_list[i]].render = lerp(array[update_list[i]].last, array[update_list[i]].actual, clamp(j, 0.0, 1.0));
          } else {
              update_list.erase(update_list.begin() + i);
              array[update_list[i]].last = array[update_list[i]].actual;
          }
        }

        
        begin_draw();
        {
            draw_all_bars();
            
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
            
            SDL_SetRenderDrawColor(sdl.m_renderer, 0, 255, 0, 255);
            SDL_RenderDrawRect(sdl.m_renderer, &selection_a);
            SDL_RenderDrawRect(sdl.m_renderer, &selection_b);
        }
        end_draw();
    }
}
//----------------------
template<std::size_t size>
void
insertionSort(std::array<value<int>, size> &array) {
    for (int i = 0; i < size; i++) {
        auto temp = array[i].actual;
        int j = i;
        
        while (temp < array[j - 1].actual && j > 0) {
            swap(array[j].actual, array[j - 1].actual, j, j - 1);
            j--;
        }
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
            
            select(i, 0, 255, 0);
            select(j, 255, 0, 0);
            select(lowest_index, 0, 255, 0);
            draw_selected();
            selected.clear();
            std::this_thread::sleep_for(50ms);
        }
        std::this_thread::sleep_for(200ms);
        swap(array[i].actual, array[lowest_index].actual, lowest_index, i);
    }
}

//----------------------
template<std::size_t size>
void
fill_array_with_rand(std::array<value<int>, size> &array) {
    for (auto &i : array) {
        i = value<int>(rand() % height);
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
    for (decltype(size) i = 0; i < size - 1; ++i) {
        for (decltype(size) j = 0; j < size - i - 1; ++j) {
            if (array[j].actual > array[j + 1].actual) {
                swap(array[j].actual, array[j + 1].actual, j, j + 1);
            }
        }
    }
}

//----------------------
void
rand_morph() {
    for (int i = 0; i < size; i++) {
        array[i].actual = (rand() % (sdl.m_renderer_height - (sdl.m_renderer_height / 5)));
        update_list.push_back(i);
    }
    
    double j = 0;
    
    while (update_list.size() > 0) {
        j += 0.01;
    
        for (int i = 0; i < update_list.size(); i++) {
            if (array[update_list[i]].render != array[update_list[i]].actual) {
                array[update_list[i]].render = lerp(array[update_list[i]].last, array[update_list[i]].actual, clamp(j, 0.0, 1.0));
            } else {
                update_list.erase(update_list.begin() + i);
                array[update_list[i]].last = array[update_list[i]].actual;
            }
        }
        
        begin_draw();
        draw_all_bars();
        end_draw();
        std::this_thread::sleep_for(20ms);
    }
    std::this_thread::sleep_for(2s);
}

//----------------------
template<typename T, std::size_t size>
void
merge(std::array<T, size> &array, decltype(size) left, decltype(size) middle, decltype(size) right) {
    
    auto n1 = middle - left + 1;
    auto n2 = right - middle;
    
    std::vector<T> left_array(n1);
    std::vector<T> right_array(n2);
    
    for (decltype(n1) i = 0; i < n1; i++) {
        left_array[i] = array[left + i];
    }
    
    for (decltype(n2) j = 0; j < n2; j++) {
        right_array[j] = array[middle + 1 + j];
    }
    
    decltype(size) i = 0, j = 0, k = left;
    
    while (i < n1 && j < n2) {
        
        if (left_array[i].actual <= right_array[j].actual) {
            array[k] = left_array[i];
            i++;
        } else {
            array[k] = right_array[j];
            j++;
        }
        ++k;
        begin_draw();
        draw_all_bars();
        end_draw();
        std::this_thread::sleep_for(50ms);
    }
    
    while (i < n1) {
        array[k] = left_array[i];
        i++;
        k++;
        begin_draw();
        draw_all_bars();
        end_draw();
        std::this_thread::sleep_for(50ms);
    }
    
    while (j < n2) {
        array[k] = right_array[j];
        j++;
        k++;
        begin_draw();
        draw_all_bars();
        end_draw();
        std::this_thread::sleep_for(50ms);
    }
}

//----------------------
template<typename T, std::size_t size>
void
merge_sort(std::array<T, size> &array, decltype(size) left, decltype(size) right) {
    if (left < right) {
        decltype(size) middle {(left + right) / 2};
        
        merge_sort(array, left, middle);
        merge_sort(array, middle + 1, right);
        merge(array, left, middle, right);
    }
}

//----------------------
template<typename T, std::size_t size>
void
merge_sort(std::array<T, size> &array) {
    merge_sort(array, 0, array.size() - 1);
}

//----------------------
int
main() {
    std::cout << "hello sailor!\n";
    std::cout << " requeted res: " << width << " x " << height << '\n';
    std::cout << "renderer res: " << sdl.m_renderer_width << " x " << sdl.m_renderer_height << '\n';
    
    srand(time(NULL));
    
    rect.y = sdl.m_renderer_height;
    rect.w = (float)sdl.m_renderer_width / (float)size;

    // show a blank screen for a while
    
    begin_draw();
    
    for (auto &i : array) {
        i.render = sdl.m_renderer_height;
        i.actual = sdl.m_renderer_height;
        i.last = sdl.m_renderer_height;
    }
    draw_all_bars();
    end_draw();
    std::this_thread::sleep_for(3s);
    
    rand_morph();
    merge_sort(array);
    rand_morph();
    selection_sort(array);
    std::this_thread::sleep_for(2s);
    rand_morph();
    insertionSort(array);
    std::this_thread::sleep_for(2s);
    rand_morph();
    bubble_sort(array);
    std::this_thread::sleep_for(2s);
   
   return 0;
}
