#ifndef NOMIC_PLUGIN_INTERFACE_H
#define NOMIC_PLUGIN_INTERFACE_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <any>
#include "nomic/core/interfaces.h"

namespace nomic {
namespace plugin {

// Forward declarations
class IPlugin;
class IPluginManager;
class IExtensionPoint;
class IPluginContext;

using PluginPtr = std::shared_ptr<IPlugin>;
using ExtensionPointPtr = std::shared_ptr<IExtensionPoint>;
using PluginContextPtr = std::shared_ptr<IPluginContext>;

/**
 * @brief Plugin metadata structure
 */
struct PluginMetadata {
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    std::vector<std::string> dependencies;
    std::vector<std::string> capabilities;
};

/**
 * @brief Main plugin interface (REQ-014: Plugin Capabilities)
 *
 * Plugins can:
 * - Add custom rules
 * - Extend DSL with new functions
 * - Add output formats
 * - Provide custom analyzers
 * - Hook into analysis pipeline
 * - Add SQL virtual tables
 */
class IPlugin {
public:
    virtual ~IPlugin() = default;

    // Lifecycle methods
    virtual bool initialize(PluginContextPtr context) = 0;
    virtual void shutdown() = 0;
    virtual bool isInitialized() const = 0;

    // Metadata
    virtual const PluginMetadata& getMetadata() const = 0;

    // Extension registration
    virtual void registerExtensions() = 0;

    // Configuration
    virtual bool configure(const std::any& config) = 0;
    virtual std::any getConfiguration() const = 0;

    // Health check
    virtual bool isHealthy() const = 0;
    virtual std::string getHealthStatus() const = 0;
};

/**
 * @brief Extension point interface for plugin system
 * Defines where plugins can hook into the system
 */
class IExtensionPoint {
public:
    virtual ~IExtensionPoint() = default;

    enum class ExtensionType {
        DSL_FUNCTION,        // Add DSL functions
        SQL_TABLE,          // Add SQL virtual tables
        RULE,               // Add analysis rules
        ANALYZER,           // Add custom analyzers
        OUTPUT_FORMAT,      // Add output formats
        PIPELINE_HOOK,      // Hook into analysis pipeline
        PATTERN_MATCHER,    // Add pattern matchers
        METRIC_CALCULATOR   // Add metric calculators
    };

    virtual ExtensionType getType() const = 0;
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;
};

/**
 * @brief Plugin context providing access to core system
 */
class IPluginContext {
public:
    virtual ~IPluginContext() = default;

    // Core system access
    virtual core::SemanticModelPtr getSemanticModel() const = 0;
    virtual core::IASTFactory* getASTFactory() const = 0;

    // Extension point registration
    virtual bool registerDSLFunction(const std::string& name,
                                    std::function<std::any(const std::vector<std::any>&)> func) = 0;
    virtual bool registerSQLTable(const std::string& tableName,
                                 std::function<void()> tableProvider) = 0;
    virtual bool registerRule(const std::string& ruleName,
                             std::function<void(core::ASTNodePtr)> ruleChecker) = 0;
    virtual bool registerAnalyzer(const std::string& name,
                                 std::function<void(core::SemanticModelPtr)> analyzer) = 0;
    virtual bool registerOutputFormat(const std::string& format,
                                     std::function<std::string(const std::any&)> formatter) = 0;
    virtual bool registerPipelineHook(const std::string& phase,
                                     std::function<void(const std::any&)> hook) = 0;

    // Service discovery
    virtual std::any getService(const std::string& serviceName) const = 0;
    virtual bool hasService(const std::string& serviceName) const = 0;

    // Logging
    virtual void logInfo(const std::string& message) const = 0;
    virtual void logWarning(const std::string& message) const = 0;
    virtual void logError(const std::string& message) const = 0;

    // Configuration access
    virtual std::any getGlobalConfig(const std::string& key) const = 0;
    virtual void setGlobalConfig(const std::string& key, const std::any& value) = 0;
};

/**
 * @brief Plugin manager interface for managing plugin lifecycle
 */
class IPluginManager {
public:
    virtual ~IPluginManager() = default;

    // Plugin management
    virtual bool loadPlugin(const std::string& path) = 0;
    virtual bool unloadPlugin(const std::string& name) = 0;
    virtual void loadAllPlugins(const std::string& directory) = 0;
    virtual void unloadAllPlugins() = 0;

    // Plugin discovery
    virtual std::vector<PluginPtr> getAllPlugins() const = 0;
    virtual PluginPtr getPlugin(const std::string& name) const = 0;
    virtual bool hasPlugin(const std::string& name) const = 0;

    // Extension point management
    virtual std::vector<ExtensionPointPtr> getExtensionPoints() const = 0;
    virtual std::vector<ExtensionPointPtr> getExtensionPointsByType(IExtensionPoint::ExtensionType type) const = 0;
    virtual ExtensionPointPtr getExtensionPoint(const std::string& name) const = 0;

    // Plugin dependency resolution
    virtual bool resolveDependencies() = 0;
    virtual std::vector<std::string> getUnresolvedDependencies() const = 0;

    // Plugin configuration
    virtual bool configurePlugin(const std::string& name, const std::any& config) = 0;
    virtual std::any getPluginConfig(const std::string& name) const = 0;

    // Health monitoring
    virtual void checkPluginHealth() = 0;
    virtual std::vector<std::pair<std::string, std::string>> getUnhealthyPlugins() const = 0;
};

/**
 * @brief Base class for plugin implementations
 * Provides common functionality for plugins
 */
class PluginBase : public IPlugin {
protected:
    PluginMetadata metadata_;
    PluginContextPtr context_;
    bool initialized_ = false;

public:
    PluginBase(const PluginMetadata& metadata) : metadata_(metadata) {}
    virtual ~PluginBase() = default;

    bool initialize(PluginContextPtr context) override {
        if (initialized_) {
            return true;
        }
        context_ = context;
        initialized_ = onInitialize();
        if (initialized_) {
            registerExtensions();
        }
        return initialized_;
    }

    void shutdown() override {
        if (!initialized_) {
            return;
        }
        onShutdown();
        initialized_ = false;
        context_.reset();
    }

    bool isInitialized() const override {
        return initialized_;
    }

    const PluginMetadata& getMetadata() const override {
        return metadata_;
    }

    bool isHealthy() const override {
        return initialized_ && context_ != nullptr;
    }

    std::string getHealthStatus() const override {
        if (!initialized_) return "Not initialized";
        if (!context_) return "No context";
        return "Healthy";
    }

protected:
    // Override these in derived classes
    virtual bool onInitialize() = 0;
    virtual void onShutdown() = 0;
};

/**
 * @brief Plugin factory function type
 * Each plugin shared library must export this function
 */
using CreatePluginFunc = std::function<PluginPtr()>;

/**
 * @brief Macro for plugin export
 * Use this in plugin implementations to export the factory function
 */
#define EXPORT_PLUGIN(PluginClass) \
    extern "C" { \
        nomic::plugin::PluginPtr createPlugin() { \
            return std::make_shared<PluginClass>(); \
        } \
    }

} // namespace plugin
} // namespace nomic

#endif // NOMIC_PLUGIN_INTERFACE_H