#include <material/material.hpp>

#include <misc/xmlutils.hpp>
#include <misc/pugixml.hpp>

#include <set>

Materials::Materials(EngineParams& params) : KernelObject(params)
{
    fprintf(stderr, "Initializing <Materials>.\n");
    fprintf(stderr, "Loading '*/materials.xml'.\n");

    std::fstream stream;
    pugi::xml_document doc;
    GetData("materials.xml", stream);
    fprintf(stderr, "Parsing materials...");

    ParseXML(doc, stream);
    fprintf(stderr, " done.\n");

    std::set<std::string> modelList;

    for (pugi::xml_node model : doc.child("materials").children("model"))
    {
        std::string modelID = model.attribute("ModelID").value();
        modelList.insert(modelID);
    }

    /* ModelID - MatID mapping. */
    std::vector<cl_uint> matMapping;
    matMapping.resize(modelList.size() + 1);
    
    /* Get the atmospheric material. */
    pugi::xml_node node = doc.child("materials").child("atmosphere");
    matMapping.push_back(node.attribute("MatID").as_uint());

    size_t index = 1;
    std::set<std::string>::iterator iter;
    for (iter = modelList.begin(); iter != modelList.end(); ++iter)
    {
        /* Map this Model ID to the desired material ID. */
        pugi::xml_node node = doc.child("materials");
        node = node.find_child_by_attribute("ModelID", (*iter).c_str());

        matMapping[index++] = node.attribute("MatID").as_uint();
    }

    mapping = CreateBuffer(params.context,
                           CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                           sizeof(cl_uint) * matMapping.size(),
                           &matMapping[0]);

    fprintf(stderr, "Initialization complete.\n\n");
}

void Materials::Bind(cl_uint* index)
{
    fprintf(stderr, "Binding <mapping@Materials> to index %u.\n", *index);
    BindArgument(params.kernel, mapping, (*index)++);
}

void Materials::Update(size_t /* index */)
{
    return;
}

void* Materials::Query(size_t /* query */)
{
    return nullptr;
}
