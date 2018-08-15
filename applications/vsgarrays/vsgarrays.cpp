#include <vsg/core/Object.h>
#include <vsg/core/Auxiliary.h>
#include <vsg/core/Array.h>
#include <vsg/core/Visitor.h>

#include <iostream>
#include <algorithm>
#include <mutex>


template<typename T>
inline std::ostream& operator << (std::ostream& output, const vsg::tvec4<T>& vec)
{
    output << vec.x << " " << vec.y<<" "<<vec.z<<" "<<vec.w;
    return output; // to enable cascading
}

struct Unique
{
    using StringIndexMap = std::map< std::string, std::size_t >;
    using IndexStringMap = std::map< std::size_t, std::string >;

    std::mutex _mutex;
    StringIndexMap _stringIndexMap;
    IndexStringMap _indexStringMap;

    std::size_t getIndex(const std::string& name)
    {
        std::lock_guard<std::mutex> guard(_mutex);
        StringIndexMap::iterator  itr = _stringIndexMap.find(name);
        if (itr != _stringIndexMap.end()) return itr->second;

        std::size_t s = _stringIndexMap.size();
        _stringIndexMap[name] = s;
        _indexStringMap[s] = name;
         return s;
    }

    std::string getName(std::size_t index)
    {
        std::lock_guard<std::mutex> guard(_mutex);
        IndexStringMap::iterator  itr = _indexStringMap.find(index);
        if (itr != _indexStringMap.end()) return itr->second;
        return std::string();
    }

};

class Attribute : public vsg::Object
{
public:
    Attribute(const std::string& name, vsg::Object* object=nullptr) : index(getIndex(name)), data(object) {}

    std::size_t               index;
    vsg::ref_ptr<vsg::Object> data;

    std::size_t getIndex() const { return index; }
    std::string getName() const { return getName(index); }

    static std::size_t getIndex(const std::string& name);
    static std::string getName(std::size_t index);
};

static Unique s_AttributeUnique;

std::size_t Attribute::getIndex(const std::string& name)
{
    return s_AttributeUnique.getIndex(name);
}

std::string Attribute::getName(std::size_t index)
{
    return s_AttributeUnique.getName(index);
}

class Uniform : public vsg::Object
{
public:
    Uniform(const std::string& name, vsg::Object* object=nullptr) : index(getIndex(name)), data(object) {}

    std::size_t               index;
    vsg::ref_ptr<vsg::Object> data;

    std::size_t getIndex() const { return index; }
    std::string getName() const { return getName(index); }

    static std::size_t getIndex(const std::string& name);
    static std::string getName(std::size_t index);
};

static Unique s_UniformUnique;

std::size_t Uniform::getIndex(const std::string& name)
{
    return s_UniformUnique.getIndex(name);
}

std::string Uniform::getName(std::size_t index)
{
    return s_UniformUnique.getName(index);
}

template<std::size_t NumUniforms, std::size_t NumAttributes>
class State : public vsg::Object
{
public:
    using Uniforms = std::array< vsg::ref_ptr<Uniform>, NumUniforms >;
    using Attributes = std::array< vsg::ref_ptr<Attribute>, NumUniforms >;

    Uniforms uniforms;
    Attributes attributes;

    void set(Uniform* uniform) { uniforms[uniform->index] = uniform; }
    void set(Attribute* attribute) { attributes[attribute->index] = attribute; }
};


int main(int /*argc*/, char** /*argv*/)
{

    vsg::ref_ptr<vsg::floatArray> floats = new vsg::floatArray(10);

    std::cout<<"floats.size() = "<<floats->size()<<std::endl;

    float value = 0.0f;
    std::for_each(floats->begin(), floats->end(), [&value](float& v) {
        v = value++;
    });

    std::for_each(floats->begin(), floats->end(), [](float& v) {
        std::cout<<"   v[] = "<<v<<std::endl;
    });

    vsg::ref_ptr<vsg::vec4Array> colours = new vsg::vec4Array();
    colours->resize(20);
    vsg::vec4 colour(0.25, 0.5, 0.75, 1.0);
    for (std::size_t i=0; i<colours->size(); ++i)
    {
        (*colours)[i] = colour;
        colour = vsg::vec4(colour.g, colour.b, colour.a, colour.r);
    }

    std::for_each(colours->begin(), colours->end(), [](vsg::vec4& c) {
        std::cout<<"   c[] = "<<c<<std::endl;
    });

    std::cout<<"colours->at(4) = "<<colours->at(4)<<std::endl;
    std::cout<<"(*colours)[5] = "<<(*colours)[5]<<std::endl;

    vsg::ref_ptr< State<10,10> > state = new State<10,10>;
    state->set(new Uniform("values", floats.get()));
    state->set(new Uniform("col", colours.get()));
    state->set(new Attribute("colours", colours.get()));


    std::cout<<"State : "<<std::endl;
    std::cout<<"Uniforms : "<<std::endl;
    std::for_each(state->uniforms.begin(), state->uniforms.end(), [](vsg::ref_ptr<Uniform>& uniform)
    {
        if (uniform.valid()) std::cout<<"    index="<<uniform->index<<", name="<<uniform->getName()<<", data="<<uniform->data.get()<<std::endl;
        else std::cout<<"    nullptr"<<std::endl;
    });

    std::cout<<"Attributes : "<<std::endl;
    std::for_each(state->attributes.begin(), state->attributes.end(), [](vsg::ref_ptr<Attribute>& attribute)
    {
        if (attribute.valid()) std::cout<<"    index="<<attribute->index<<", name="<<attribute->getName()<<", data="<<attribute->data.get()<<std::endl;
        else std::cout<<"    nullptr"<<std::endl;
    });

    vsg::ref_ptr<vsg::vec2Array> texCoords = new vsg::vec2Array
    {
        {1.0f, 2.0f},
        {3.0f, 4.0f},
        {}
    };

    std::cout<<"texCoords.size() = "<<texCoords->size()<<std::endl;
    for(auto p : *texCoords)
    {
        std::cout<<"    tc "<<p.x<<", "<<p.y<<std::endl;
    }

    vsg::ref_ptr<vsg::vec4Array> col = new vsg::vec4Array{{}};
    std::cout<<"col.size() = "<<col->size()<<std::endl;
    for(auto c : *col)
    {
        std::cout<<"    colour "<<c.r<<", "<<c.g<<", "<<c.b<<", "<<c.a<<std::endl;
    }

    return 0;
}
