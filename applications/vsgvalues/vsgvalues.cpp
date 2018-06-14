#include <vsg/core/Object.h>
#include <vsg/core/Auxiliary.h>
#include <vsg/core/Value.h>
#include <vsg/core/Visitor.h>

#include <iostream>
#include <typeinfo>

int main(int argc, char** argv)
{
    vsg::ref_ptr<vsg::Object> object = new vsg::Object;
    object->setValue("name", std::string("Name field contents"));
    object->setValue("time", 10.0);
    object->setValue("count", -5);

    if (object->getAuxiliary())
    {
        struct VisitValues : public vsg::Visitor
        {
            void apply(vsg::Object& object)
            {
                std::cout<<"Object, "<<typeid(object).name()<<std::endl;
            }

            void apply(vsg::IntValue& value)
            {
                std::cout<<"FloatValue,  value = "<<value.value<<std::endl;
            }

            void apply(vsg::FloatValue& value)
            {
                std::cout<<"FloatValue, value  = "<<value.value<<std::endl;
            }

            void apply(vsg::DoubleValue& value)
            {
                std::cout<<"DoubleValue, value  = "<<value.value<<std::endl;
            }

            void apply(vsg::StringValue& value)
            {
                std::cout<<"StringValue, value  = "<<value.value<<std::endl;
            }
        };

        VisitValues visitValues;

        std::cout<<"Object has Auxiliary so check it's ObjectMap for our values. "<<object->getAuxiliary()<<std::endl;
        for(vsg::Auxiliary::ObjectMap::iterator itr = object->getAuxiliary()->getObjectMap().begin();
            itr != object->getAuxiliary()->getObjectMap().end();
            ++itr)
        {
            std::cout<<"   key["<<itr->first.name<<", "<<itr->first.index<<"] ";
            itr->second->accept(visitValues);
        }
    }



    return 0;
}