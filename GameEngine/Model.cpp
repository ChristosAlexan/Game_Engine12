
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "Model.h"
#include <iostream>
#include "ErrorLogger.h"
#include "Vertex.h"
#include "MeshData.h"
#include "mikktspace.h"


int GetNumFaces(const SMikkTSpaceContext* context)
{
    auto* mesh = reinterpret_cast<MikkMeshWrapper*>(context->m_pUserData);
    return static_cast<int>(mesh->indices->size() / 3);
}

int GetNumVerticesOfFace(const SMikkTSpaceContext* context, int face)
{
    return 3; // always triangles
}

void GetPosition(const SMikkTSpaceContext* context, float posOut[3], int face, int vert)
{
    auto* mesh = reinterpret_cast<MikkMeshWrapper*>(context->m_pUserData);
    const uint32_t index = (*mesh->indices)[face * 3 + vert];
    const Vertex& v = (*mesh->vertices)[index];
    posOut[0] = v.pos.x;
    posOut[1] = v.pos.y;
    posOut[2] = v.pos.z;
}

void GetNormal(const SMikkTSpaceContext* context, float normOut[3], int face, int vert)
{
    auto* mesh = reinterpret_cast<MikkMeshWrapper*>(context->m_pUserData);
    const uint32_t index = (*mesh->indices)[face * 3 + vert];
    const Vertex& v = (*mesh->vertices)[index];
    normOut[0] = v.normal.x;
    normOut[1] = v.normal.y;
    normOut[2] = v.normal.z;
}

void GetTexCoord(const SMikkTSpaceContext* context, float uvOut[2], int face, int vert)
{
    auto* mesh = reinterpret_cast<MikkMeshWrapper*>(context->m_pUserData);
    const uint32_t index = (*mesh->indices)[face * 3 + vert];
    const Vertex& v = (*mesh->vertices)[index];
    uvOut[0] = v.texCoord.x;
    uvOut[1] = v.texCoord.y;
}

void SetTangentSpace(const SMikkTSpaceContext* pContext, const float tangent[3], float fSign, int face, int vert)
{
    MikkMeshWrapper* wrapper = static_cast<MikkMeshWrapper*>(pContext->m_pUserData);
    int index = (*wrapper->indices)[face * 3 + vert];

    (*wrapper->vertices)[index].tangent = DirectX::XMFLOAT4(tangent[0], tangent[1], tangent[2], tangent[3]);

    // Optional: compute binormal if needed
    if (fSign < 0.0f)
    {
        const DirectX::XMFLOAT3& normal = (*wrapper->vertices)[index].normal;
        DirectX::XMVECTOR n = DirectX::XMLoadFloat3(&normal);
        DirectX::XMVECTOR t = DirectX::XMLoadFloat4(&(*wrapper->vertices)[index].tangent);
        DirectX::XMVECTOR b = DirectX::XMVector3Cross(n, t);
        b = DirectX::XMVectorScale(b, fSign);
        DirectX::XMStoreFloat3(&(*wrapper->vertices)[index].binormal, b);
    }
}

DirectX::XMMATRIX Node::getLocalMatrix()
{
    DirectX::XMVECTOR trans_vec = DirectX::XMLoadFloat3(&translation);
    DirectX::XMVECTOR rot_vec = DirectX::XMLoadFloat4(&rotation);
    DirectX::XMVECTOR scale_vec = DirectX::XMLoadFloat3(&scale);
    DirectX::XMMATRIX T = DirectX::XMMatrixTranslationFromVector(trans_vec);
    DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(rot_vec);
    DirectX::XMMATRIX S = DirectX::XMMatrixScalingFromVector(scale_vec);

    return  S * R * T ;
}


Model::Model()
{
}

bool Model::LoadModel(const std::string& filepath)
{
    uint32_t animIndex = 0;
    models.resize(m_animFiles.size() + 1);
    bool ret;

    for (int i = 0; i < models.size(); ++i)
    {
        std::string _path;

        if (i == 0)
            _path = filepath;
        else
            _path = m_animFiles[animIndex++];

        if (_path.find(".glb") != std::string::npos)
            ret = loader.LoadBinaryFromFile(&models[i], &err, &warn, _path);
        else if (_path.find(".gltf") != std::string::npos)
            ret = loader.LoadASCIIFromFile(&models[i], &err, &warn, _path);

        if (!ret)
        {
            ErrorLogger::Log("Failed to load: " + _path);
            return false;
        }
    }
   
    auto scene = models[0].scenes[0];
    for (size_t i = 0; i < scene.nodes.size(); i++)
    {
        const tinygltf::Node node = models[0].nodes[scene.nodes[i]];
        LoadNode(node, models[0], nullptr, scene.nodes[i]);
    }

    // Generate tangents
    SMikkTSpaceInterface mikktInterface = {};
    mikktInterface.m_getNumFaces = GetNumFaces;
    mikktInterface.m_getNumVerticesOfFace = GetNumVerticesOfFace;
    mikktInterface.m_getPosition = GetPosition;
    mikktInterface.m_getNormal = GetNormal;
    mikktInterface.m_getTexCoord = GetTexCoord;
    mikktInterface.m_setTSpaceBasic = SetTangentSpace;
   
    MikkMeshWrapper wrapper = { &m_cpuMesh.vertices, &m_cpuMesh.indices };
   
    SMikkTSpaceContext context = {};
    context.m_pInterface = &mikktInterface;
    context.m_pUserData = &wrapper;
   
    genTangSpaceDefault(&context);

    LoadSkeleton(models[0]);

    for (auto& model : models)
    {
        LoadAnimations(model);
    }
}

void Model::SetAnimFiles(const std::vector<std::string>& animFiles)
{
    m_animFiles = animFiles;
}


ECS::MeshData& Model::GetMeshData()
{
    return m_cpuMesh;
}


void Model::LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent, uint32_t nodeIndex)
{
    Node* node = new Node{};
    node->matrix = DirectX::XMMatrixIdentity();
    node->parent = parent;
    node->index = nodeIndex;
    node->skin = inputNode.skin;
    totalNodeCount++;

    if (inputNode.translation.size() == 3)
    {
        node->translation =
            DirectX::XMFLOAT3(inputNode.translation[0], inputNode.translation[1], inputNode.translation[2]);
    }
    if (inputNode.scale.size() == 3)
    {
        node->scale =
            DirectX::XMFLOAT3(inputNode.scale[0], inputNode.scale[1], inputNode.scale[2]);
    }
    if (inputNode.rotation.size() == 4)
    {
        node->rotation =
            DirectX::XMFLOAT4(inputNode.rotation[0], inputNode.rotation[1], inputNode.rotation[2], inputNode.rotation[3]);
    }

    // Load node's children
    if (inputNode.children.size() > 0) 
    {
        for (size_t i = 0; i < inputNode.children.size(); i++) 
        {
            LoadNode(input.nodes[inputNode.children[i]], input, node, inputNode.children[i]);
        }
    }
    // If the node contains mesh data, we load vertices and indices from the buffers
    // In glTF this is done via accessors and buffer views
    if (inputNode.mesh > -1) 
    {
        const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];
        // Iterate through all primitives of this node's mesh
        for (size_t i = 0; i < mesh.primitives.size(); i++) 
        {
            const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
            uint32_t firstIndex = static_cast<uint32_t>(m_cpuMesh.indices.size());
            uint32_t vertexStart = static_cast<uint32_t>(m_cpuMesh.vertices.size());
            uint32_t indexCount = 0;
            // Vertices
            {
                const float* positionBuffer = nullptr;
                const float* normalsBuffer = nullptr;
                const float* texCoordsBuffer = nullptr;
                const float* weightsBuffer = nullptr;
                const uint16_t* jointsBuffer = nullptr;
                const float* tangentsBuffer = nullptr;
                size_t vertexCount = 0;

                // Get buffer data for vertex positions
                if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("POSITION")->second];
                    const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                    positionBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    vertexCount = accessor.count;
                }
                // Get buffer data for vertex normals
                if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
                    const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                    normalsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }
                // Get buffer data for vertex texture coordinates
                // glTF supports multiple sets, we only load the first one
                if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
                    const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                    texCoordsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }
                if (glTFPrimitive.attributes.find("WEIGHTS_0") != glTFPrimitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("WEIGHTS_0")->second];
                    const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                    weightsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }
                if (glTFPrimitive.attributes.find("JOINTS_0") != glTFPrimitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("JOINTS_0")->second];
                    const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                    jointsBuffer = reinterpret_cast<const uint16_t*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }
                if (glTFPrimitive.attributes.find("TANGENT") != glTFPrimitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TANGENT")->second];
                    const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                    tangentsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                // Append data to model's vertex buffer
                for (size_t vertexOffset = 0; vertexOffset < vertexCount; ++vertexOffset) {
                    Vertex vert{};
                    vert.pos = positionBuffer ? DirectX::XMFLOAT3(positionBuffer[0 + (vertexOffset * 3)], positionBuffer[1 + (vertexOffset * 3)], positionBuffer[2 + (vertexOffset * 3)]) : DirectX::XMFLOAT3(0, 0, 0);
                    vert.normal = normalsBuffer ? DirectX::XMFLOAT3(normalsBuffer[0 + (vertexOffset * 3)], normalsBuffer[1 + (vertexOffset * 3)], normalsBuffer[2 + (vertexOffset * 3)]) : DirectX::XMFLOAT3(0,0,0);
                    vert.texCoord = texCoordsBuffer ? DirectX::XMFLOAT2(texCoordsBuffer[0 + (vertexOffset * 2)], texCoordsBuffer[1 + (vertexOffset * 2)]) : DirectX::XMFLOAT2(0, 0);


                    


                    if (weightsBuffer)
                    {
                        vert.boneWeights[0] = weightsBuffer[0 + (vertexOffset * 4)];
                        vert.boneWeights[1] = weightsBuffer[1 + (vertexOffset * 4)];
                        vert.boneWeights[2] = weightsBuffer[2 + (vertexOffset * 4)];
                        vert.boneWeights[3] = weightsBuffer[3 + (vertexOffset * 4)];
                    }
                    else
                    {
                        vert.boneWeights[0] = 0.0f;
                        vert.boneWeights[1] = 0.0f;
                        vert.boneWeights[2] = 0.0f;
                        vert.boneWeights[3] = 0.0f;
                    }
                  
                    if(jointsBuffer)
                    {
                        vert.boneIndices[0] = jointsBuffer[0 + (vertexOffset * 4)];
                        vert.boneIndices[1] = jointsBuffer[1 + (vertexOffset * 4)];
                        vert.boneIndices[2] = jointsBuffer[2 + (vertexOffset * 4)];
                        vert.boneIndices[3] = jointsBuffer[3 + (vertexOffset * 4)];
                    }
                    else
                    {

                        vert.boneIndices[0] = 0;
                        vert.boneIndices[1] = 0;
                        vert.boneIndices[2] = 0;
                        vert.boneIndices[3] = 0;
                    }

                   if (tangentsBuffer)
                   {
                       using namespace DirectX;
                       vert.tangent.x = tangentsBuffer[0 + (vertexOffset * 4)];
                       vert.tangent.y = tangentsBuffer[1 + (vertexOffset * 4)];
                       vert.tangent.z = tangentsBuffer[2 + (vertexOffset * 4)];
                       vert.tangent.w = tangentsBuffer[3 + (vertexOffset * 4)];
                       // generate binormals(bitangents)
                       DirectX::XMVECTOR tangent = DirectX::XMLoadFloat4(&vert.tangent);
                       DirectX::XMVector3Normalize(tangent);
                       DirectX::XMVECTOR binormal = DirectX::XMLoadFloat3(&vert.binormal);
                       DirectX::XMVECTOR normal = DirectX::XMLoadFloat3(&vert.normal);
                   
                       binormal = DirectX::XMVector3Cross(normal, tangent) * vert.tangent.w;
                   }
                 
                   m_cpuMesh.vertices.push_back(vert);
                }
            }

            // Indices
            {
                const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.indices];
                const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];

                indexCount += static_cast<uint32_t>(accessor.count);

                // glTF supports different component types of indices
                switch (accessor.componentType) {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                    const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t index = 0; index < accessor.count; index++) {
                        m_cpuMesh.indices.push_back(buf[index] + vertexStart);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                    const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t index = 0; index < accessor.count; index++) {
                        m_cpuMesh.indices.push_back(buf[index] + vertexStart);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                    const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t index = 0; index < accessor.count; index++) {
                        m_cpuMesh.indices.push_back(buf[index] + vertexStart);
                    }
                    break;
                }
                default:
                    std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                    return;
                }
            }
            Primitive primitive{};
            primitive.firstIndex = firstIndex;
            primitive.indexCount = indexCount;
            primitive.materialIndex = glTFPrimitive.material;
            node->mesh.primitives.push_back(primitive);
        }
    }

    if (parent) {
        parent->children.push_back(node);
    }
    else {
        nodes.push_back(node);
    }
}

void Model::LoadSkeleton(tinygltf::Model& input)
{
    if (input.skins.empty())
        return;

    skins.resize(input.skins.size());

    for (size_t i = 0; i < input.skins.size(); i++)
    {
        OutputDebugStringA((input.skins[i].name + "\n").c_str());
        tinygltf::Skin glTFSkin = input.skins[i];

        skins[i].name = glTFSkin.name;
        // Find the root node of the skeleton
        skins[i].skeletonRoot = NodeFromIndex(glTFSkin.skeleton);

        // Find joint nodes
        for (int jointIndex : glTFSkin.joints)
        {
            Node* node = NodeFromIndex(jointIndex);
            if (node)
            {
                skins[i].joints.push_back(node);
            }
        }

        // Get the inverse bind matrices from the buffer associated to this skin
        if (glTFSkin.inverseBindMatrices > -1)
        {
            const tinygltf::Accessor& accessor = input.accessors[glTFSkin.inverseBindMatrices];
            const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];
            skins[i].inverseBindMatrices.resize(accessor.count);
            memcpy(skins[i].inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(DirectX::XMMATRIX));
        }
    }
}

void Model::LoadAnimations(tinygltf::Model& input)
{
    animations.push_back(Animation{});

    uint32_t index = animations.size() - 1;
    for (size_t i = 0; i < input.animations.size(); i++)
    {
        animations[index].start = std::numeric_limits<float>::max();
        animations[index].end = std::numeric_limits<float>::lowest();

        tinygltf::Animation glTFAnimation = input.animations[i];
        animations[index].name = glTFAnimation.name;

        // Samplers
        animations[index].samplers.resize(glTFAnimation.samplers.size());
        for (size_t j = 0; j < glTFAnimation.samplers.size(); j++)
        {
            tinygltf::AnimationSampler glTFSampler = glTFAnimation.samplers[j];
            AnimationSampler& dstSampler = animations[index].samplers[j];
            dstSampler.interpolation = glTFSampler.interpolation;

            // Read sampler keyframe input time values
            {
                const tinygltf::Accessor& accessor = input.accessors[glTFSampler.input];
                const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];
                const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
                const float* buf = static_cast<const float*>(dataPtr);
                for (size_t index = 0; index < accessor.count; index++)
                {
                    dstSampler.inputs.push_back(buf[index]);
                }
                // Adjust animation's start and end times
                for (auto input : animations[index].samplers[j].inputs)
                {
                    if (input < animations[index].start)
                    {
                        animations[index].start = input;
                    };
                    if (input > animations[index].end)
                    {
                        animations[index].end = input;
                    }
                }
            }

            // Read sampler keyframe output translate/rotate/scale values
            {
                const tinygltf::Accessor& accessor = input.accessors[glTFSampler.output];
                const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];
                const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
                switch (accessor.type)
                {
                case TINYGLTF_TYPE_VEC3: {
                    auto buf = static_cast<const DirectX::XMFLOAT3*>(dataPtr);

                    for (size_t index = 0; index < accessor.count; index++)
                    {
                        DirectX::XMFLOAT4 buf4(buf[index].x, buf[index].y, buf[index].z, 0.0f);
                        DirectX::XMVECTOR _vec = DirectX::XMLoadFloat4(&buf4);
                        dstSampler.outputsVec4.push_back(_vec);
                    }
                    break;
                }
                case TINYGLTF_TYPE_VEC4: {
                    auto buf = static_cast<const DirectX::XMFLOAT4*>(dataPtr);
                    for (size_t index = 0; index < accessor.count; index++)
                    {
                        DirectX::XMVECTOR _vec = DirectX::XMLoadFloat4(&buf[index]);
                        dstSampler.outputsVec4.push_back(_vec);
                    }
                    break;
                }
                default: {
                    std::cout << "unknown type" << std::endl;
                    break;
                }
                }
            }
        }

        // Channels
        animations[index].channels.resize(glTFAnimation.channels.size());
        for (size_t j = 0; j < glTFAnimation.channels.size(); j++)
        {
            tinygltf::AnimationChannel glTFChannel = glTFAnimation.channels[j];
            AnimationChannel& dstChannel = animations[index].channels[j];
            dstChannel.path = glTFChannel.target_path;
            dstChannel.samplerIndex = glTFChannel.sampler;
            dstChannel.node = NodeFromIndex(glTFChannel.target_node);
        }
    }
}

Node* Model::FindNode(Node* parent, uint32_t index)
{
    Node* nodeFound = nullptr;
    if (parent->index == index)
    {
        return parent;
    }
    for (auto& child : parent->children)
    {
        nodeFound = FindNode(child, index);
        if (nodeFound)
        {
            break;
        }
    }
    return nodeFound;
}

Node* Model::NodeFromIndex(uint32_t index)
{
    Node* nodeFound = nullptr;
    for (auto& node : nodes)
    {
        nodeFound = FindNode(node, index);
        if (nodeFound)
        {
            break;
        }
    }
    return nodeFound;
}

DirectX::XMMATRIX Model::GetNodeMatrixFlat(int nodeIndex, const std::vector<FlatNode>& flatNodes)
{
    DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();
    int current = nodeIndex;

    while (current >= 0)
    {
        const FlatNode& node = flatNodes[current];

        DirectX::XMVECTOR T = DirectX::XMLoadFloat3(&node.translation);
        DirectX::XMVECTOR R = DirectX::XMLoadFloat4(&node.rotation);
        DirectX::XMVECTOR S = DirectX::XMLoadFloat3(&node.scale);

        DirectX::XMMATRIX local =
            DirectX::XMMatrixScalingFromVector(S) *
            DirectX::XMMatrixRotationQuaternion(R) *
            DirectX::XMMatrixTranslationFromVector(T);

        matrix = matrix * local;
        current = node.parentIndex;
    }

    return matrix;
}

DirectX::XMMATRIX Model::GetNodeMatrix(Node* node)
{
    DirectX::XMMATRIX   nodeMatrix = node->getLocalMatrix();
    Node* currentParent = node->parent;
    while (currentParent)
    {
        nodeMatrix = nodeMatrix * currentParent->getLocalMatrix();
        currentParent = currentParent->parent;
    }
    return nodeMatrix;
}


void Model::UpdateAnimationBlend(float deltaTime, AnimatorComponent& animData)
{
    if (animData.currentAnim > static_cast<uint32_t>(animations.size()) - 1)
    {
        //ErrorLogger::Log("No animation with index " + animData.currentAnim);
        return;
    }

    Animation& animation = animations[animData.currentAnim];

    animData.currentTime += deltaTime;
    if (animData.currentTime > animation.end)
    {
        animData.currentTime -= animation.end;
    }

     for (auto& channel : animation.channels)
     {
         uint32_t channelNodeIndex = channel.node->index;
         auto& flatNode = animData.flatNodes[channelNodeIndex];

         AnimationSampler& sampler = animation.samplers[channel.samplerIndex];
         for (size_t i = 0; i < sampler.inputs.size() - 1; i++)
         {
             // Get the input keyframe values for the current time stamp
             if ((animData.currentTime >= sampler.inputs[i]) && (animData.currentTime <= sampler.inputs[i + 1]))
             {
                 float a = (animData.currentTime - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
                 if (channel.path == "translation")
                 {
                     if (sampler.interpolation == "STEP")
                         DirectX::XMStoreFloat3(&flatNode.translation, sampler.outputsVec4[i]);
                     else
                         DirectX::XMStoreFloat3(&flatNode.translation, DirectX::XMVectorLerp(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], a));
                 }
                 if (channel.path == "rotation")
                 {
                     if (sampler.interpolation == "STEP")
                         DirectX::XMStoreFloat4(&flatNode.rotation, sampler.outputsVec4[i]);
                     else
                     {
                         DirectX::XMVECTOR q1;
                         q1 = sampler.outputsVec4[i];
                         DirectX::XMVECTOR q2;
                         q2 = sampler.outputsVec4[i + 1];

                         DirectX::XMVECTOR rot = DirectX::XMQuaternionSlerp(q1, q2, a);
                         rot = DirectX::XMQuaternionNormalize(rot);
                         DirectX::XMStoreFloat4(&flatNode.rotation, rot);
                     }

                 }
                 if (channel.path == "scale")
                 {
                     if (sampler.interpolation == "STEP")
                         DirectX::XMStoreFloat3(&flatNode.scale, sampler.outputsVec4[i]);
                     else
                         DirectX::XMStoreFloat3(&flatNode.scale, DirectX::XMVectorLerp(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], a));
                 }

             }
         }
     }

     if (animData.currentAnim != animData.previousAnim)
     {
         if (!animData.isBlending)
         {
             animData.blendTime = 0.0f;
             animData.isBlending = true;
         }
     }
     // Blend previous and current animation
     if (animData.isBlending)
     {
         animData.blendTime += deltaTime;
         float t = animData.blendTime / animData.blendDuration;
         float smoothT = (1.0f - cosf(t * DirectX::XM_PI)) * 0.5f; // smoothstep for better blending between animations
         t = std::clamp(t, 0.0f, 1.0f);

         Animation& animationPrev = animations[animData.previousAnim];
         for (int i = 0; i < animData.flatNodes.size(); ++i)
         {
             auto& curNode = animData.flatNodes[i];
             auto& prevNode = animData.flatNodesPrev[i];

             DirectX::XMVECTOR currentNodePos = DirectX::XMLoadFloat3(&curNode.translation);
             DirectX::XMVECTOR prevNodePos = DirectX::XMLoadFloat3(&prevNode.translation);
             DirectX::XMStoreFloat3(&curNode.translation, DirectX::XMVectorLerp(prevNodePos, currentNodePos, smoothT));

             DirectX::XMVECTOR currentNodeRot = DirectX::XMLoadFloat4(&curNode.rotation);
             DirectX::XMVECTOR prevNodeRot = DirectX::XMLoadFloat4(&prevNode.rotation);
             DirectX::XMVECTOR rot = DirectX::XMQuaternionSlerp(prevNodeRot, currentNodeRot, smoothT);
             rot = DirectX::XMQuaternionNormalize(rot);
             DirectX::XMStoreFloat4(&curNode.rotation, rot);

             DirectX::XMVECTOR currentNodeScale = DirectX::XMLoadFloat3(&curNode.scale);
             DirectX::XMVECTOR prevNodeScale = DirectX::XMLoadFloat3(&prevNode.scale);
             DirectX::XMStoreFloat3(&curNode.scale, DirectX::XMVectorLerp(prevNodeScale, currentNodeScale, smoothT));
         }

         if (t >= 1.0f)
         {
             animData.isBlending = false;
             animData.previousAnim = animData.currentAnim;
             animData.flatNodesPrev = animData.flatNodes;
             animData.blendTime = 0.0f;
         }

     }
    
    for (auto& node : nodes)
    {
        UpdateJointsFlat(node, animData);
    }
}

void Model::UpdateAnimation(float deltaTime, AnimatorComponent& animData)
{
    if (animData.currentAnim > static_cast<uint32_t>(animations.size()) - 1)
    {
        //ErrorLogger::Log("No animation with index " + animData.currentAnim);
        return;
    }

    Animation& animation = animations[animData.currentAnim];
    animData.currentTime += deltaTime;
    if (animData.currentTime > animation.end)
    {
        animData.currentTime -= animation.end;
    }

    for (auto& channel : animation.channels)
    {
        AnimationSampler& sampler = animation.samplers[channel.samplerIndex];
        for (size_t i = 0; i < sampler.inputs.size() - 1; i++)
        {
            // Get the input keyframe values for the current time stamp
            if ((animData.currentTime >= sampler.inputs[i]) && (animData.currentTime <= sampler.inputs[i + 1]))
            {
                float a = (animData.currentTime - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
                if (channel.path == "translation")
                {
                    if (sampler.interpolation == "STEP")
                        DirectX::XMStoreFloat3(&channel.node->translation, sampler.outputsVec4[i]);
                    else
                        DirectX::XMStoreFloat3(&channel.node->translation, DirectX::XMVectorLerp(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], a));
                }
                if (channel.path == "rotation")
                {
                    if (sampler.interpolation == "STEP")
                        DirectX::XMStoreFloat4(&channel.node->rotation, sampler.outputsVec4[i]);
                    else
                    {
                        DirectX::XMVECTOR q1;
                        q1 = sampler.outputsVec4[i];
                        DirectX::XMVECTOR q2;
                        q2 = sampler.outputsVec4[i + 1];

                        DirectX::XMVECTOR rot = DirectX::XMQuaternionSlerp(q1, q2, a);
                        rot = DirectX::XMQuaternionNormalize(rot);
                        DirectX::XMStoreFloat4(&channel.node->rotation, rot);
                    }

                }
                if (channel.path == "scale")
                {
                    if (sampler.interpolation == "STEP")
                        DirectX::XMStoreFloat4(&channel.node->rotation, sampler.outputsVec4[i]);
                    else
                        DirectX::XMStoreFloat3(&channel.node->scale, DirectX::XMVectorLerp(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], a));
                }

            }
        }
    }
    for (auto& node : nodes)
    {
        UpdateJointsFlat(node, animData);
    }
}



void Model::UpdateJoints(Node* node, std::vector<DirectX::XMMATRIX>& finalTransform)
{
    if (node->skin > -1) {

        Skin& skin = skins[node->skin];
        //DirectX::XMMATRIX inverseTransform = DirectX::XMMatrixInverse(nullptr, GetNodeMatrix(node));

        size_t numJoints = skin.joints.size();
        finalTransform.resize(numJoints);
  
        for (size_t i = 0; i < numJoints; i++)
        {
            finalTransform[i] = skin.inverseBindMatrices[i] * GetNodeMatrix(skin.joints[i]) ;
            //finalTransform[i] = finalTransform[i] * inverseTransform ;

            finalTransform[i] = DirectX::XMMatrixTranspose(finalTransform[i]);
        }
    }

    for (auto& child : node->children)
    {
        UpdateJoints(child, finalTransform);
    }
}

void Model::UpdateJointsFlat(Node* node, AnimatorComponent& animData)
{

    if (node->skin > -1) {

        Skin& skin = skins[node->skin];
        //DirectX::XMMATRIX inverseTransform = DirectX::XMMatrixInverse(nullptr, GetNodeMatrix(node));

        size_t numJoints = skin.joints.size();
        animData.finalTransforms.resize(numJoints);

        for (size_t i = 0; i < numJoints; i++)
        {
            animData.finalTransforms[i] = skin.inverseBindMatrices[i] * GetNodeMatrixFlat(skin.joints[i]->index, animData.flatNodes);
            //finalTransform[i] = finalTransform[i] * inverseTransform ;

            animData.finalTransforms[i] = DirectX::XMMatrixTranspose(animData.finalTransforms[i]);
        }
    }

    for (auto& child : node->children)
    {
        UpdateJointsFlat(child, animData);
    }
}

void Model::CalculateFinalTransform(float dt, AnimatorComponent& animData)
{
    UpdateAnimation(dt, animData);
}

void Model::CalculateFinalTransformBlend(float dt, AnimatorComponent& animData)
{
    UpdateAnimationBlend(dt, animData);
}

void Model::BuildFlatHierarchy(AnimatorComponent& animData)
{
    if (!animData.flatNodes.empty())
        return;

    animData.flatNodes.resize(totalNodeCount);
    for (Node* root : nodes)
    {
        FlattenHierarchyRecursive(root, -1, animData); // parent index = -1 for root
    }

    animData.flatNodesPrev = animData.flatNodes;
}

void Model::FlattenHierarchyRecursive(Node* node, int parentIndex, AnimatorComponent& animData)
{
    int index = node->index; // already assigned during LoadNode

    FlatNode& flat = animData.flatNodes[index];
    flat.translation = node->translation;
    flat.rotation = node->rotation;
    flat.scale = node->scale;
    flat.parentIndex = parentIndex;

    for (Node* child : node->children)
    {
        FlattenHierarchyRecursive(child, index, animData);
    }
}
