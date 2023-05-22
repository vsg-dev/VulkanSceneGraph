
#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>
#include <vsg/utils/SharedObjects.h>

using namespace vsg;

SharedObjects::SharedObjects()
{
    suitableForSharing = SuitableForSharing::create();
}

SharedObjects::~SharedObjects()
{
}

bool SharedObjects::suitable(const Path& filename) const
{
    return excludedExtensions.count(vsg::lowerCaseFileExtension(filename)) == 0;
}

bool SharedObjects::contains(const Path& filename, ref_ptr<const Options> options) const
{
    std::scoped_lock<std::recursive_mutex> lock(_mutex);

    auto loadedObject_id = std::type_index(typeid(LoadedObject));
    auto itr = _sharedObjects.find(loadedObject_id);
    if (itr == _sharedObjects.end()) return false;

    auto& loadedObjects = itr->second;
    auto key = LoadedObject::create(filename, options);
    return loadedObjects.find(key) != loadedObjects.end();
}

void SharedObjects::add(ref_ptr<Object> object, const Path& filename, ref_ptr<const Options> options)
{
    std::scoped_lock<std::recursive_mutex> lock(_mutex);

    auto loadedObject_id = std::type_index(typeid(LoadedObject));
    auto& loadedObjects = _sharedObjects[loadedObject_id];

    auto key = LoadedObject::create(filename, options, object);
    loadedObjects.insert(key);
}

bool SharedObjects::remove(const Path& filename, ref_ptr<const Options> options)
{
    std::scoped_lock<std::recursive_mutex> lock(_mutex);

    auto loadedObject_id = std::type_index(typeid(LoadedObject));
    auto itr = _sharedObjects.find(loadedObject_id);
    if (itr == _sharedObjects.end()) return false;

    auto& loadedObjects = itr->second;

    auto key = LoadedObject::create(filename, options);
    if (auto lo_itr = loadedObjects.find(key); lo_itr != loadedObjects.end())
    {
        loadedObjects.erase(lo_itr);
        return true;
    }
    else
    {
        return false;
    }
}

void SharedObjects::clear()
{
    std::scoped_lock<std::recursive_mutex> lock(_mutex);
    _defaults.clear();
    _sharedObjects.clear();
}

void SharedObjects::prune()
{
    std::scoped_lock<std::recursive_mutex> lock(_mutex);

    auto loadedObject_id = std::type_index(typeid(LoadedObject));

    // record observer pointers for each LoadedObject object so we can clear them to prevent local references keeping them from being pruned
    auto& loadedObjects = _sharedObjects[loadedObject_id];
    std::vector<observer_ptr<Object>> observedLoadedObjects(loadedObjects.size());
    auto observedLoadedObject_itr = observedLoadedObjects.begin();
    for (auto& object : loadedObjects)
    {
        auto& loadedObject = static_cast<LoadedObject&>(*object);
        *(observedLoadedObject_itr++) = loadedObject.object;
        loadedObject.object = {};
    }

    // record observer pointers for each shared default object so we can clear them to prevent local references keeping them from being pruned
    std::vector<observer_ptr<Object>> observedDefaults(_defaults.size());
    auto observedDefaults_itr = observedDefaults.begin();
    for (auto defaults_itr = _defaults.begin(); defaults_itr != _defaults.end(); ++defaults_itr)
    {
        *(observedDefaults_itr++) = defaults_itr->second;
    }
    _defaults.clear();

    // prune SharedObjects that don't have an external references (referenceCount != 1)
    bool prunedObjects = false;
    do
    {
        prunedObjects = false;
        for (auto itr = _sharedObjects.begin(); itr != _sharedObjects.end(); ++itr)
        {
            auto id = itr->first;
            if (id != loadedObject_id)
            {
                auto& objects = itr->second;
                for (auto object_itr = itr->second.begin(); object_itr != itr->second.end();)
                {
                    if ((*object_itr)->referenceCount() == 1)
                    {
                        object_itr = objects.erase(object_itr);
                        prunedObjects = true;
                    }
                    else
                    {
                        ++object_itr;
                    }
                }
            }
        }
    } while (prunedObjects);

    observedLoadedObject_itr = observedLoadedObjects.begin();
    for (auto object_itr = loadedObjects.begin(); object_itr != loadedObjects.end();)
    {
        auto& loadedObject = static_cast<LoadedObject&>(*(*object_itr));
        loadedObject.object = *(observedLoadedObject_itr++);
        if (!loadedObject.object)
        {
            object_itr = loadedObjects.erase(object_itr);
        }
        else
        {
            ++object_itr;
        }
    }

    // reassign any default objects that still have references
    for (auto& observerDefault : observedDefaults)
    {
        ref_ptr<Object> defaultObject = observerDefault;
        if (defaultObject)
        {
            auto& object = *defaultObject;
            _defaults[std::type_index(typeid(object))] = defaultObject;
        }
    }
}

void SharedObjects::report(std::ostream& out)
{
    std::scoped_lock<std::recursive_mutex> lock(_mutex);
    out << "SharedObjects::report(..) " << this << std::endl;
    out << "SharedObjects::_defaults " << _defaults.size() << std::endl;
    for (auto& [type, object] : _defaults)
    {
        out << "    " << type.name() << ", object = " << object << " " << object->referenceCount() << std::endl;
    }

    out << "SharedObjects::_sharedObjects " << SharedObjects::_sharedObjects.size() << std::endl;
    for (auto& [type, objects] : _sharedObjects)
    {
        out << "    " << type.name() << ", objects = " << objects.size() << std::endl;
        for (auto& object : objects)
        {
            out << "        object = " << object << " "
                << " " << object->referenceCount() << std::endl;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// LoadedObject
//
LoadedObject::LoadedObject(const Path& in_filename, ref_ptr<const Options> in_options, ref_ptr<Object> in_object) :
    filename(in_filename),
    options(Options::create_if(in_options, *in_options)),
    object(in_object)
{
    if (options) options->sharedObjects = {};
}

void LoadedObject::traverse(Visitor& visitor)
{
    if (object) object->accept(visitor);
}
void LoadedObject::traverse(ConstVisitor& visitor) const
{
    if (object) object->accept(visitor);
}

int LoadedObject::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = filename.compare(rhs.filename))) return result;
    return compare_pointer(options, rhs.options);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SuitableForSharing
//
void SuitableForSharing::apply(const Object& object)
{
    if (suitableForSharing) object.traverse(*this);
}

void SuitableForSharing::apply(const PagedLOD&)
{
    suitableForSharing = false;
}
